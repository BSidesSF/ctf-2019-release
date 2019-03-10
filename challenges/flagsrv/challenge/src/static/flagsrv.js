(function() {
  var powBits = 12;
  var powField = '__pow__';
  var generatePowValue = async function(provenValue) {
    var importParams = {name: 'HMAC', hash: 'SHA-256'};
    var randBytes = new ArrayBuffer(32);
    var viewBytes = new Uint8Array(randBytes);
    var dataBuf = new TextEncoder("utf-8").encode(provenValue);
    while(true) {
      crypto.getRandomValues(viewBytes);
      // make to hex
      var hexString = '';
      for(var i=0; i<viewBytes.length/2; i++) {
        var x = ('0' + viewBytes[i*2].toString(16)).slice(-2);
        hexString += x;
        viewBytes[i*2] = x.charCodeAt(0);
        viewBytes[i*2+1] = x.charCodeAt(1);
      }
      var key = await crypto.subtle.importKey(
        'raw', randBytes, importParams, true, ['sign', 'verify']);
      var sig = await crypto.subtle.sign('HMAC', key, dataBuf);
      sig = new Uint8Array(sig);
      // Check prefix length
      if (verifySig(sig)) {
        return hexString;
      }
    }
  };
  var verifySig = function(sig) {
    var count = 0;
    for (var i=0; i<sig.length; i++) {
      if (sig[i] == 0) {
        count += 8;
        continue;
      }
      for (var k=7;k>=0;k--) {
        if ((sig[i] & (1 << k)) != 0)
          break;
        count++;
      }
      break;
    }
    return count >= powBits;
  };
  window.GPV = generatePowValue;
  var updatePowField = async function(formname, fields) {
    var form = document.getElementById(formname);
    var provenString = '';
    fields.sort();
    for(var i=0;i<fields.length;i++) {
      if (i > 0) provenString += ';';
      try{
        var field = form.elements[fields[i]];
        provenString += field.value;
      }catch{}
    }
    var start = getMs();
    var powVal = await generatePowValue(provenString);
    var end = getMs();
    console.log('POW took ' + (end-start) + ' ms');
    form.elements[powField].value = powVal;
  }
  var getMs = function() {
    return (new Date()).getTime();
  };
  var retrieveKey = async function(u) {
    var buf = new ArrayBuffer(u.length);
    var v = new Uint8Array(buf);
    for (var i=0; i<u.length; i++) {
      v[i] = u.charCodeAt(i);
    }
    var k = await crypto.subtle.importKey(
      'raw',
      buf,
      {name: 'HMAC', hash: 'SHA-256', length: u.length*8},
      false,
      ['sign', 'verify']);
    var k2 = await crypto.subtle.sign(
      'HMAC',
      k,
      buf);
    return base32encode(k2);
  };
  var base32encode = function(ab) {
    var alphabet = '';
    for (var i='A'.charCodeAt(0); i<='Z'.charCodeAt(0); i++) {
      alphabet += String.fromCharCode(i);
    }
    for (var i='2'.charCodeAt(0); i<='7'.charCodeAt(0); i++) {
      alphabet += String.fromCharCode(i);
    }
    ab = new Uint8Array(ab);
    var len = ab.length;
    var offlen = len % 5;
    var pads = Array(0, 6, 4, 3, 1);
    var padlen = pads[offlen];
    var inb = new ArrayBuffer(len + padlen);
    var inarr = new Uint8Array(inb);
    inarr.set(ab);
    var outab = new ArrayBuffer(Math.ceil(len/5) * 8);
    var outarr = new Uint8Array(outab);
    var encodeChunk = function(chunk, out) {
      var bits = new Uint8Array(new ArrayBuffer(40));
      for (var i=0; i<chunk.length; i++) {
        for (var k=7; k>=0; k--) {
          var v;
          if ((chunk[i] & (1 << k)) == 0)
            v = 0;
          else
            v = 1;
          bits[i*8+(7-k)] = v;
        }
      }
      for (var i=0; i<bits.length; i+=5) {
        var piece = bits.slice(i, i+5);
        c = ((piece[0] << 4) |
             (piece[1] << 3) |
             (piece[2] << 2) |
             (piece[3] << 1) |
              piece[4]);
        out[i/5] = alphabet.charCodeAt(c);
      }
    };
    for (var v=0; v<len/5; v++) {
      var out = outarr.subarray(v*8, (v+1)*8);
      encodeChunk(inarr.slice(v*5, (v+1)*5), out);
    }
    return String.fromCharCode.apply(null, outarr.slice(0, outarr.length - padlen));
  };
  var b32encodestr = function(s) {
    var buf = new Uint8Array(new ArrayBuffer(s.length));
    for(var i=0;i<s.length;i++)
      buf[i] = s.charCodeAt(i);
    return base32encode(buf);
  };
  var handleLoginPages = function() {
    var fname = 'loginform';
    var form = document.getElementById(fname);
    if (form !== null) {
      var fields = Array('username', 'password');
      if (form.action.endsWith('/2fa')) {
        fields = Array('username', 'passcode');
      }
      form.addEventListener('submit', async function(evt) {
        evt.preventDefault();
        await updatePowField(fname, fields);
        form.submit();
      });
    }
  };
  var handleEnrollPage = async function() {
    var mfaDiv = document.getElementById('setupMFA');
    if (mfaDiv === null) {
      return;
    }
    var b32key = await retrieveKey(mfaDiv.dataset.username);
    document.getElementById('secret').textContent = b32key;
    var uri = ('otpauth://totp/Flag%20Server:' +
      encodeURIComponent(mfaDiv.dataset.username) +
      '?secret=' + b32key
    );
    document.getElementById('secretUri').textContent = uri;
    var code = new QRCode(document.getElementById('qrcode'),
      {
        text: uri,
        height: 200,
        width: 200,
      });
  };
  window.b32encodestr = b32encodestr;
  window.base32encode = base32encode;
  var addHandlers = function() {
    handleLoginPages();
    handleEnrollPage();
  };
  document.addEventListener("DOMContentLoaded", function(event) {
    addHandlers();
  });
}());
