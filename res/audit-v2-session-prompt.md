[MODEL SELECTION]

The current model is the main orchestrator. Delegate when useful, prefer the cheapest capable model, parallelize independent tasks, and escalate only when needed.

Use models roughly as follows:

- Fast/cheap: gemini-3.5-flash-extra-low, gemini-3.1-flash-lite, gemini-3-flash, gemini-3.5-flash-low, claude-3-5-haiku, claude-haiku-4-5, gpt-5.4-mini, gpt-5.3-codex-spark, gpt-5.6-luna. Best for small edits, searches, summaries, tests, and simple fixes. Weakest on architecture and difficult debugging.
- Balanced: claude-3-7-sonnet, claude-sonnet-4, claude-sonnet-4-5, claude-sonnet-4-6, claude-sonnet-5, gpt-5.4, gpt-5.5, gpt-5.6-terra, gemini-3.1-pro-low, gemini-pro-agent, gemini-3-flash-agent, gpt-oss-120b-medium. Best for implementation, debugging, repository analysis, and review. More expensive than lightweight models.
- Deep reasoning: gpt-5.6-sol, claude-opus-4, claude-opus-4-1, claude-opus-4-5, claude-opus-4-6, claude-opus-4-6-thinking, claude-opus-4-7, claude-opus-4-8. Best for architecture, hard bugs, ambiguous problems, and final verification. Slowest and most expensive.
- Specialized: codex-auto-review for code review; claude-fable-5 for writing, naming, and documentation; gpt-image-1.5, gpt-image-2, and gemini-3.1-flash-image for visual tasks.

Prefer lightweight models first. Use stronger models only when the task, risk, or failed attempts justify them. The orchestrator is responsible for integration, verification, and final correctness.

Use lightweight models for cheap subtasks, Sonnet-class models for substantial implementation, and Sol or Opus only for difficult or high-risk problems.

Subagents may act as implementers, researchers, reviewers, or editors.

As editors, subagents should:

- Inspect existing code before making changes.
- Preserve the current architecture, behavior, and coding style.
- Make the smallest complete change that solves the assigned problem.
- Remove duplication, dead code, unclear naming, and unnecessary complexity.
- Improve readability without introducing unrelated refactors.
- Check interfaces, callers, tests, documentation, and nearby assumptions.
- Report questionable code instead of silently changing uncertain behavior.
- Return a concise summary, exact files changed, risks, and verification results.

The orchestrator should give every editor:

- A narrow objective and explicit boundaries.
- The relevant files, constraints, and acceptance criteria.
- Permission to modify only the assigned scope.
- A required output format containing patches, findings, tests, and uncertainties.

The orchestrator remains responsible for resolving conflicting edits, integrating patches, running final verification, and ensuring correctness.

[SPECIFIC INFO]

GPT models    are provided by ONE OpenAI Plus Subscription    => All share a weekly limit.
Claude models are provided by TWO Anthropic Pro Subscriptions => All share 5-hourly limits and a weekly limit.
Gemini models are provided by ONE free Antigravity account    => Free but with limited quota.

Choose the option which minimizes cost and maximizes longevity

[STRICT CODE STYLE]
- max <=80 chars/line
- file header comments only, style as existing code
- NO comments inside functions
- no unused variables
- no silenced compiler warnings
- use descriptive variable names, even in macros
- -> and : in Params and Return section pairs must align
- use `///` for doc comments and `/*---*\` `\*---*/` comments to make code readable
- ALL structs, enums, and function hoisting should be in header files; source files should only contain function implementation and variable declarations
- Non-source documents are exempt from source line-width rules.
- Keep existing source/header file count unchanged. Do not add source modules or architecture headers.
- After every header enum, struct, or prototype change, run clean debug/release builds. Makefile header dependencies are not trusted.

[CONTEXT]

This is a deckbuilder roguelike game. `res/GDD.md` is mechanics source of truth. Everything named there is actual game content. Do not invent cards, pieces, relics, places, events, bosses, artifacts, or mechanics outside GDD.

Repository: `/Users/aldenluthfi/Documents/Projects/daulat`

Important files:

- `res/audit-plan-v2.md`: authoritative F0.5-F10 and S1-S9 execution plan.
- `res/audit-baseline-v2.md`: runtime baseline, GDD matrix, fixture contracts.
- `res/audit-v2-phases/README.md`: current phase chain.
- `res/audit-v2-phases/<PHASE>.md`: current/prior phase receipt.
- `res/audit-v2-phases/TEMPLATE.md`: required phase receipt format.
- `res/analysis.md` and `res/audit-plan-v1.md`: user-owned reference only; never edit them.

Engine invariants:

- `x` stays mutable concrete effect payload. Never add universal wrapper value.
- Operation metadata belongs in frame/accessor state, not `x`.
- Generic engine code names operations/phases, never item identities.
- Preserve behavior and ordering unless GDD correction requires change.
- Crusade is only approved intentional redesign. GDD defect fixes are corrections, not redesigns.
- Do not begin simplification until F10 behavior freeze passes.

[SESSION PHASE]

Current phase: `<PHASE>`

This session owns exactly `<PHASE>`. It may not start a later F/S phase.

Before editing:

1. Recall any memories you have to understand the design philosophy of the game.
2. Read `res/audit-v2-phases/README.md`.
3. Read previous completed receipts, if any.
4. Read `<PHASE>` section in `res/audit-plan-v2.md`.
5. Read matching fixture contract in `res/audit-baseline-v2.md`.
6. Read relevant GDD and source.
7. Check `git status`; preserve all pre-existing user changes.

If current receipt is `IN PROGRESS` or `BLOCKED`, resume `<PHASE>` only. Do not re-plan architecture or skip to another phase.

[MAIN GOAL]

Execute audit-v2 one phase per full session:

```text
F0/F0.5 baseline and fixture routes
F1-F10 feature completion and behavior freeze
S1-S9 simplification after F10 only
```

Read all the previous receipts before this one (`<PHASE>`).

Feature phases complete all GDD-backed effect composition before refactor. Simplification phases reduce duplicated plumbing without behavior loss or new source/header files.

[PLAN EXPECTATION]

- Treat `res/audit-plan-v2.md` as execution specification, not a starting point for another broad plan.
- Use smallest rollback-safe implementation boundaries inside `<PHASE>`.
- Maintain task tracking for implementation, runtime evidence, and review.
- Add/update only one receipt for this phase: `res/audit-v2-phases/<PHASE>.md`.
- Do not create blank receipts for future phases.
- `COMPLETE` requires every gate named in plan. Build success or source reading alone never completes a phase.
- If unresolved: write `IN PROGRESS` or `BLOCKED` in same receipt, include exact blocker, and next session resumes same phase.
- Counsel requires user decision before F4: discard leaves two cards versus deterministic refill. Do not choose or implement it without approval.
- Surface any other genuine GDD contradiction; do not invent behavior.

[IMPLEMENTATION EXPECTATION]

- Match surrounding C idiom and strict code style.
- Place declarations, structs, enums, and public prototypes in `incl/representation.h`.
- Do not alter `res/analysis.md` or `res/audit-plan-v1.md`.
- Do not commit, push, reset, or discard unrelated work unless explicitly asked.
- Do not use unit tests, direct internal calls, test-only engine hooks, or patched end state as behavior proof.

Verification uses `bin/daulat` public stdin/stdout protocol:

1. Create normal temporary profile with `new` then `save`.
2. Patch only legal persistent starting inputs in isolated save: masteries, seed, difficulty, challenge, unlock bits, Vorath, battles, chains, liberation, synergies, and map state.
3. Load through public `load`.
4. Drive real state transition through protocol commands.
5. Capture decisive app output and at least one adjacent negative probe.
6. Run relevant grep/static gates.
7. Run `make clean && make debug` and `make clean && make release` after header changes; otherwise run builds required by phase plan.
8. Add exact commands/results to this phase receipt and baseline evidence.

Before ending session, receipt must state:

- phase status and source/header count before/after;
- files changed and behavior decisions;
- public runtime setup, commands, output, probes, and save/load evidence;
- build/static results;
- gate result and unresolved blockers;
- explicit statement no downstream phase began;
- only allowed next phase plus copy-ready next-session directive.

[START DIRECTIVE]

Execute `<PHASE>` only. Read its receipt, plan section, fixture contract, GDD, and relevant source. Preserve existing work. Do not begin later phases.
