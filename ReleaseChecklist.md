  1. Update VERSION and CHANGELOG files.
  1. Disable OBS auto-publish.
  1. Check SPEC file section for file list changes.
  1. Run `obs-staging` script.
  1. Upload staged files to OBS: `osc addremove && osc ci`
  1. Test packages.
  1. Tag release in Mercurial.
  1. Upload source tarball from staging to Google Code.
  1. Update home page (current release number).
  1. Add RSS feed entry.
  1. Enable OBS auto-publish.
  1. Update changelog, version, and source download link on kde-look.org.
  1. Update changelog, version, and source download link on kde-apps.org.