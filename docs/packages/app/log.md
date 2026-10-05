<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# app:log

app:log — thread-safe logging from any worker, flushed at the barrier.

    import "app:log"

    void panel_job(^app.JobArgs a):
        log.info("worker", "chunk done", a.worker)

    void process(f32 dt):
        app.parallel_for(n, grain, panel_job, data)   // joins before returning
        log.flush()                                    // safe: no job is in flight

**The mechanism.** `core:log` is the fixed `Record` shape and the
sink contract; this is the opinion on top — where records live between
being logged and being seen, and when that happens. Each worker gets its
own fixed-capacity ring (`MAX_WORKERS`, `core:jobs`' own bound), indexed
by `JobArgs.worker`. A log call from inside a job writes only to that
worker's ring — no lock, no other worker's memory touched, no mid-line
interleave, because nobody else can be writing there.

**Why no lock is safe, not just fast.** A
ring is not read until `flush`, and `flush` is the caller's to call —
after every job that might log this frame has been joined (`parallel_for`
already joins before returning; a caller using `submit`/`wait` directly
must `wait` first). That is the same temporal split `app:app.tick`
already relies on for `jobs.reset_scratch`: workers only run *during* the
parallel phase, `flush` only runs *after* it, so the two never overlap
and no atomic is needed on the ring itself.

**Default worker is 0** (`i32 worker = 0`, an ordinary default parameter)
— the main thread's own index in `core:jobs`' scheme, so ordinary
non-job code (`ready`, `process`, `shutdown`) calls `log.info(tag, msg)`
with nothing extra to say. Only a call from inside a `JobFn` needs to
name its own worker, because that is the one piece of context this
package cannot infer on its own (Dado has no thread-local storage surface).

**Overflow drops the newest**, the same stance `app:input`'s ring takes —
P1/P2 baseline, no backpressure. A frame that logs more than `RING_CAP`
times from one worker is a sign something is spamming, not a case worth
spending a resize on.

**The stdout sink is on from the first call, with no setup** — it works
from the first frame onward. `add_sink` registers more.

**This package imports no UI, deliberately (`CLASSIC-B`, 2026-09-06).** It
used to carry `set_ui_panel`/`panel_fn`, a sink that painted flushed records
into the old UI's `Paragraph` control — so the *logging* package imported a
*UI* package, which is the dependency edge pointing the wrong way. That UI
has been deleted along with the rest of the TUI, and the sink went with it
rather than being repointed at `app:ui`, for two reasons stated here so
the next person does not have to rediscover them:

  1. Repointing would have moved the inversion, not removed it — `app:log`
     would still `import "app:ui"`.
  2. It does not need to live here. `add_sink` takes a `corelog.Sink`, which
     is a plain `(fn, rawptr)` pair, so a program that wants its log on
     screen writes a sink of its own that appends each record to lines it
     keeps, and shows those lines in an `app:ui` text view whose source
     reads them. The UI knowledge stays on the UI side of the boundary.

## Declarations

0 declarations, 0 public.

