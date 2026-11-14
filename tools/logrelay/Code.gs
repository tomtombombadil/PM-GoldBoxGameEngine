// Gold Box Game Engine - log relay (Google Apps Script web app).
//
// The port's Send Logs posts its log files here as ordinary form fields; this
// script zips them and emails them to the logs mailbox it runs in. If the
// player gave a reply address it becomes the email's Reply-To, and the player
// gets one fixed thank-you note asking what they were doing.
//
// Deploy: Deploy > New deployment > Web app, Execute as: Me, Who has access:
// Anyone. Nothing here is secret: the web address only lets a caller trigger
// what this file does. Limits below keep a stranger from doing more than
// leaving a little junk in this one mailbox.

const TO = 'cyd.classic.games.logs@gmail.com';   // the mailbox this script runs in
const MAX_UPLOAD = 3 * 1024 * 1024;              // bytes, the whole upload
const MAX_FILES = 12;
const MAX_REPORTS_PER_DAY = 60;                  // emails to the mailbox
const MAX_THANKS_PER_DAY = 20;                   // notes to players (one per address per day)

function doGet() {
  return text('Gold Box log relay is running.');
}

function doPost(e) {
  try {
    const p = (e && e.parameter) || {};
    if (p.app !== 'goldbox') return text('ERROR not a Gold Box log upload');
    if (e.postData && e.postData.length > MAX_UPLOAD) return text('ERROR too big');

    // The files: fields named "file:<name>"
    const blobs = [];
    Object.keys(p).sort().forEach(function (k) {
      if (k.indexOf('file:') !== 0 || blobs.length >= MAX_FILES) return;
      const name = k.substring(5).replace(/[^A-Za-z0-9._-]/g, '_').substring(0, 60);
      if (name && p[k]) blobs.push(Utilities.newBlob(p[k], 'text/plain', name));
    });
    if (!blobs.length) return text('ERROR no log files');

    const info = {
      version: clean(p.version), device: clean(p.device), firmware: clean(p.firmware),
      portmaster: clean(p.portmaster), screen: clean(p.screen)
    };
    const reply = address(p.reply);
    const day = today();

    // Today's counts (one script property; a lock so two uploads don't trip each other)
    const lock = LockService.getScriptLock();
    lock.waitLock(20000);
    let thank = false;
    try {
      const props = PropertiesService.getScriptProperties();
      let c = JSON.parse(props.getProperty('counts') || '{}');
      if (c.day !== day) c = { day: day, reports: 0, thanks: 0, who: [] };
      if (c.reports >= MAX_REPORTS_PER_DAY) return text('ERROR busy - try again tomorrow');
      c.reports++;
      if (reply && c.thanks < MAX_THANKS_PER_DAY) {
        const id = digest(reply);
        if (c.who.indexOf(id) < 0) {
          c.who.push(id);
          c.thanks++;
          thank = true;
        }
      }
      props.setProperty('counts', JSON.stringify(c));
    } finally {
      lock.releaseLock();
    }
    if (MailApp.getRemainingDailyQuota() < 2) return text('ERROR busy - try again tomorrow');

    const stamp = Utilities.formatDate(new Date(), 'Etc/UTC', 'yyyyMMdd-HHmmss');
    const zip = Utilities.zip(blobs, 'goldbox-logs_' + stamp + '_v' + (info.version || 'x') + '.zip');
    const lines = [
      'Gold Box Game Engine: ' + (info.version ? 'v' + info.version : 'unknown version'),
      'Device: ' + (info.device || 'unknown'),
      'Firmware: ' + (info.firmware || 'unknown'),
      'PortMaster: ' + (info.portmaster || 'unknown'),
      'Screen: ' + (info.screen || 'unknown'),
      'Reply address: ' + (reply || 'none given'),
      'Files: ' + blobs.map(function (b) { return b.getName(); }).join(', '),
      '',
      '--- the end of log.txt ---',
      tail(p['file:log.txt'], 40)
    ];
    const mail = {
      to: TO,
      name: 'Gold Box Logs',
      subject: ('Gold Box logs v' + (info.version || '?') + ' - ' + (info.device || 'unknown device') + ' - ' +
                (info.firmware || 'unknown firmware') + (reply ? ' - ' + reply : '')).substring(0, 240),
      body: lines.join('\n'),
      attachments: [zip]
    };
    if (reply) mail.replyTo = reply;
    MailApp.sendEmail(mail);

    if (thank) {
      MailApp.sendEmail({
        to: reply,
        name: 'Gold Box Game Engine',
        replyTo: TO,
        subject: 'We got your Gold Box Game Engine logs',
        body: 'Thank you - the logs you sent from the Gold Box Game Engine arrived.\n\n' +
              'Logs only tell part of the story. If you can, please reply to this message and tell us:\n' +
              '  - which game you were playing, and where in it you were\n' +
              '  - what you were doing just before the problem\n' +
              '  - what went wrong, and what you expected instead\n\n' +
              'You are getting this note because this address was typed into Send Logs on a handheld. ' +
              'If that was not you, just ignore it - nothing more will be sent.\n'
      });
    }
    return text('OK ' + blobs.length + ' files' + (thank ? ' thanks' : ''));
  } catch (err) {
    return text('ERROR ' + String(err).substring(0, 120));
  }
}

function text(s) {
  return ContentService.createTextOutput(s).setMimeType(ContentService.MimeType.TEXT);
}

// One line, printable, 80 characters at most
function clean(s) {
  return String(s || '').replace(/[^\x20-\x7E]/g, ' ').replace(/\s+/g, ' ').trim().substring(0, 80);
}

// A plausible email address, or ''
function address(s) {
  s = String(s || '').trim();
  return s.length <= 100 && /^[A-Za-z0-9._%+-]+@[A-Za-z0-9-]+(\.[A-Za-z0-9-]+)+$/.test(s) ? s : '';
}

function digest(s) {
  return Utilities.base64Encode(Utilities.computeDigest(Utilities.DigestAlgorithm.SHA_256, s.toLowerCase())).substring(0, 16);
}

function today() {
  return Utilities.formatDate(new Date(), 'Etc/UTC', 'yyyy-MM-dd');
}

function tail(s, n) {
  if (!s) return '(no log.txt)';
  const a = String(s).split('\n');
  return a.slice(Math.max(0, a.length - n)).join('\n');
}
