# Website And GitHub Pages

Use this reference for website content, visual QA, SEO, or deployment.

## Source And Deployment Contract

- Author website changes in `docs/` on `codex/persist-tchinese-ui-settings`.
- Deploy the same tree to the root of `gh-pages`.
- Keep `gw2.jakeuj.com` in `CNAME`; preserve `.nojekyll`, `robots.txt`, `sitemap.xml`, canonical URLs, and relative asset paths.
- Do not assume pushing the maintained branch updates production. Confirm the remote `gh-pages` commit and the live custom domain separately.
- Do not add analytics, third-party runtime JavaScript, tracking fonts, or unverified artwork.

Keep player-facing facts synchronized with `README.md` and the current stable release:

- stable fork version and download URL;
- DLL location from `addons/arcdps/arcdps.log`, including root and `bin64` possibilities without duplicate copies;
- `addons/arcdps/arcdps_tchineseui.ini`;
- `Shift + Alt + T`;
- healthy and degraded log behavior;
- backup and rollback steps;
- upstream author, MIT License, media attribution, and third-party disclaimer.

Verify mutable version and release facts before editing. Do not infer the latest stable tag from a test tag.

## Validation

For design or implementation work, also use the available `build-gw2-websites` skill. Before deployment, run:

```bash
python /Users/jakeuj/.codex/skills/build-gw2-websites/scripts/audit_gw2_site.py . --site-dir docs
node --check docs/script.js
xmllint --noout docs/sitemap.xml
git diff --check
```

Use a controllable browser for visual QA. At minimum verify:

- 1440×900, 768×1024, and 390×844 with no horizontal overflow;
- mobile menu, Escape close, focus restoration, skip link, scroll-spy, video behavior, and console errors;
- `prefers-reduced-motion`, no JavaScript, missing `IntersectionObserver`, and coarse pointer fallbacks;
- first-fold image budget and local asset availability.

Do not claim the native DLL was built when only the website changed.

## Safe Publication

Require an explicit publish request. Keep the maintained worktree unchanged by deploying through a temporary `gh-pages` worktree:

```bash
git status --short --branch
git fetch origin
deploy_worktree=$(mktemp -d /tmp/gw2-arcdps-pages.XXXXXX)
git worktree add "$deploy_worktree" origin/gh-pages
rsync -a --delete --exclude=.git docs/ "$deploy_worktree"/
git -C "$deploy_worktree" status --short --branch
git -C "$deploy_worktree" diff --check
```

Before committing, verify the temporary tree contains the expected `CNAME`, new page marker, local scripts, and assets. Then commit and push only the deployment tree:

```bash
git -C "$deploy_worktree" add --all
git -C "$deploy_worktree" commit -m "Deploy project site"
git -C "$deploy_worktree" push origin HEAD:gh-pages
```

If `gh-pages` advanced after the worktree was created, fetch and reconcile it instead of force-pushing. Never force-push the deployment branch by default.

After a successful push, poll `https://gw2.jakeuj.com/` with a cache-busting query until the expected new marker appears. Verify `script.js` and critical assets return HTTP 200. Treat the deployment as complete only after the custom domain serves the new content. Remove the temporary worktree with `git worktree remove <path>`.
