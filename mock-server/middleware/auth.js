/**
 * HTTP Basic Authentication middleware
 * Mimics ESP32's request->authenticate() behavior
 */

function basicAuth(mockData) {
  return (req, res, next) => {
    const authHeader = req.headers.authorization;

    if (!authHeader) {
      res.setHeader('WWW-Authenticate', 'Basic realm="ESP32 Login"');
      return res.status(401).send('Authentication required');
    }

    const auth = Buffer.from(authHeader.split(' ')[1], 'base64').toString().split(':');
    const username = auth[0];
    const password = auth[1];

    if (username === mockData.webUser && password === mockData.webPassword) {
      next();
    } else {
      res.setHeader('WWW-Authenticate', 'Basic realm="ESP32 Login"');
      return res.status(401).send('Invalid credentials');
    }
  };
}

module.exports = basicAuth;
