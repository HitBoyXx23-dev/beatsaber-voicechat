// Vercel serverless function: GET /api/version
// Lets the mod (and the site) check the latest released build.
// Stateless on purpose, so it runs on Vercel without any database.
const LATEST = {
  gameVersion: '1.40.8',
  modVersion: '1.0.0',
  qmod: '/downloads/VoiceChat-1.40.8.qmod',
};

module.exports = (req, res) => {
  res.setHeader('Content-Type', 'application/json');
  res.setHeader('Cache-Control', 'public, max-age=300');
  if (req.method !== 'GET') {
    res.status(405).json({ error: 'method not allowed' });
    return;
  }
  res.status(200).json({ ...LATEST, ok: true });
};
