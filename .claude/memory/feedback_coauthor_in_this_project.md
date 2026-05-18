---
name: Co-Authored-By trailer IS used in this project
description: In this project (trees / ft5-pmr-probe context), Co-Authored-By IS included per codestyle.org Agentic Instructions
type: feedback
originSessionId: febb364f-d8bd-42ed-a1af-9c37e8675e7e
---
In this project, the user explicitly said "General rules still apply, following codestyle.org" at the start of the campaign.

The `docs/codestyle.org` Agentic Instructions section says:
> Add a Co-Authored-By trailer to every commit and merge message:
> Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>

**Why:** User explicitly invoked codestyle.org, which mandates the trailer.

**How to apply:** Include the Co-Authored-By trailer in every commit message in this project, using `git commit -m "$(cat <<'EOF' ... Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com> EOF)"` pattern.

Note: The general feedback memory (feedback_no_coauthor.md) says "don't add unless explicitly asked" — this project is the exception where it was explicitly requested via codestyle.org invocation.
