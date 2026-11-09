// dados/io.js: where a library's scripts write. `print` and `println` are what a
// Dado host provides natively; a package built as a JavaScript library has no Dado
// host, so each class module imports them from here. Each takes one string, which
// the script rendered. Under a host with a standard output (node, Deno's and
// Bun's `process`) the text is written as it is; elsewhere (a browser page) each
// finished line goes to `console.log`.

const stdout = typeof process === "object" && process !== null && typeof process.stdout === "object" &&
    process.stdout !== null && typeof process.stdout.write === "function" ? process.stdout : null;

// The text of the line `print` has started and not finished, without a stdout.
let pending = "";

export function print(s) {
    if (stdout !== null) {
        stdout.write(s);
        return;
    }
    pending += s;
    let i = pending.indexOf("\n");
    while (i >= 0) {
        console.log(pending.slice(0, i));
        pending = pending.slice(i + 1);
        i = pending.indexOf("\n");
    }
}

export function println(s) {
    print(s + "\n");
}
