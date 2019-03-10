
-- sqlite
CREATE TABLE users (
  username TEXT NOT NULL PRIMARY KEY,
  password TEXT NOT NULL,
  twofactor INTEGER NOT NULL,
  flag TEXT,
  readonly INTEGER);

-- mysql
CREATE TABLE users (
  username VARCHAR(255) NOT NULL PRIMARY KEY,
  password VARCHAR(255) NOT NULL,
  twofactor TINYINT NOT NULL,
  flag VARCHAR(255),
  readonly TINYINT);

INSERT INTO users(username, password, twofactor, flag, readonly) VALUES(
  'flag', 'eed3OhY3UteegohngaQuahgh', 1, 'CTF{two_factor_more_like_no_factor}', 1);

