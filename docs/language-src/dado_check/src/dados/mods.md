//@ toys/dialect 35
//@ ### Loading a script by its path
//@
//@ A script may load another script that the program was not compiled with
//@ — a mod — by its path, at run time: `any g = Script.load("mods/goblin",
//@ self)` reads `mods/goblin.js` under the working directory, the mod's
//@ JavaScript, which `dadoc --emit=mod` wrote, and constructs its class, its
//@ init method given the arguments after the path. The mod runs in the same
//@ engine as the script that loaded it, and stopping or deleting that
//@ script's engine ends it too. Each load reads the file afresh.
//@
//@ A mod sees only what it is handed. It imports nothing but the runtime
//@ every script imports: a mod that imports Dado — a `dado:` module, or a
//@ package's script — or JavaScript fails at load, and the `Error` names the
//@ import. So the API a mod has is the objects its loader hands its init
//@ method; `dadoc --emit=script-api` writes the classes of a package a mod
//@ is compiled against.
//@
//@ What `Script.load` answers is an `any`. A script calls it as it calls
//@ any untyped value, converting each answer where it is used, or takes it
//@ as a class of its own program — `Enemy e = g` — which checks the mod's
//@ object at that line: every method of `Enemy` but init must be there with
//@ as many parameters, and every field there too. From then on each
//@ method's answer and each field read is checked as it arrives, as any
//@ value leaving an `any` is. A load that fails — no file, a module that
//@ does not compile, an import it may not have, an init method that throws
//@ — raises an `Error` the script may catch.
