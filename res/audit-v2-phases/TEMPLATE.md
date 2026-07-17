# <PHASE> — Audit v2 phase receipt

## Status

- **Phase:** `<PHASE>`
- **Status:** `IN PROGRESS | BLOCKED | COMPLETE`
- **Date:** `YYYY-MM-DD`
- **Session scope:** `<one sentence>`
- **Source/header count:** `<before> -> <after>; must be unchanged>`

## Entry contract

- Prior receipt: `<path or none>`
- Plan section: `res/audit-plan-v2.md:<line>`
- Fixture contract: `res/audit-baseline-v2.md:<line>`
- GDD scope: `<citations>`
- No downstream phase began: `<yes/no; explain>`

## Implementation

| File | Change | Reason |
|---|---|---|
| `<path>` | `<change>` | `<phase requirement>` |

## Behavior decisions

- `<GDD-backed decision or approved intentional change>`
- `<ordering/lifetime/composition invariant>`

## Runtime evidence

### Fixture setup

```text
<normal new/save; allowed temporary save patch; load>
```

### Protocol commands

```text
<real commands>
```

### Observed output

```text
<decisive app output>
```

### Probes

- `<negative/adjacent protocol probe and observed result>`

## Static and build gates

```text
<grep/line-count checks>
make clean && make debug
make clean && make release
```

Results:

- `<exact result; warnings/errors if any>`

## Gate result

- `<each phase-gate requirement: pass/fail/blocker>`

## Blockers and deferred work

- `<none, or exact blocker>`

## Next session

- **Allowed next phase:** `<same phase, or next phase only when COMPLETE>`
- **Fresh-session directive:**

```text
Resume <PHASE> only. Read this receipt, current plan section, F0.5 route
contract, GDD, and relevant source. Do not begin a later phase.
```
