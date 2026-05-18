---
name: No co-author in commits
description: Do not add Co-Authored-By lines to commit messages unless explicitly asked
type: feedback
originSessionId: fc6534d9-7465-4826-9c89-6178c86470d6
---
When working under codestyle.org rules (e.g., user says "following codestyle.org"), include Co-Authored-By trailers as that document specifies them. In sessions where codestyle.org is NOT explicitly invoked, omit Co-Authored-By.

**Why:** codestyle.org's Agentic Instructions mandate the trailer. The user may invoke or suppress this depending on context.

**How to apply:** Check whether the user's instructions for the session reference codestyle.org. If yes, include the trailer per that document. If no, omit it.
