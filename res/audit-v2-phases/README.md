# Audit v2 phase receipts

One receipt exists for each started F/S phase. A receipt is a handoff record,
not a replacement for `res/audit-plan-v2.md` or `res/audit-baseline-v2.md`.

## Rules

- Start exactly one named F/S phase per full session.
- Read prior completed receipt, current phase in `res/audit-plan-v2.md`, F0.5
  route contract, GDD, and relevant source before edits.
- Create a receipt when phase starts or ends; update same file while blocked.
- `COMPLETE` requires every phase gate and public-protocol evidence.
- `IN PROGRESS` or `BLOCKED` resumes same phase next session. Never skip ahead.
- Do not pre-create blank files for future phases.
- Receipts are non-source artifacts; source/header file count remains fixed.

## Current chain

| Phase | Status | Receipt | Next |
|---|---|---|---|
| F0 | COMPLETE | `F0.md` | F0.5 complete |
| F0.5 | COMPLETE | `F0.5.md` | F1 |
| F1 | COMPLETE | `F1.md` | F2 |
| F2 | COMPLETE | `F2.md` | F3 |
| F3 | COMPLETE | `F3.md` | F4 |
| F4 | COMPLETE | `F4.md` | F5 |
| F5 | COMPLETE | `F5.md` | F6 |
| F6 | COMPLETE | `F6.md` | F7 |
| F7 | NOT STARTED | none | Gold chain, recipe forbid |

## Fresh-session entry

Copy `res/audit-v2-session-prompt.md`, replace `<PHASE>` with `F7`, and include
any current receipt if the phase is resumed.
