#!/bin/sh
# Installs Dado:  curl -fsSL https://raw.githubusercontent.com/dadolangcom/Dado/main/install.sh | sh
#
# Downloads the release archive for this machine, checks it against the
# release's SHA256SUMS, unpacks it into ~/dado (or --dir), and runs the
# install.sh inside it, which runs `dadoc bootstrap`. Options:
#
#   --dir <folder>     install there instead of ~/dado
#   --version <x.y.z>  that release instead of the latest
#
# With no --version it takes the latest stable release, and while there is none
# (every release so far a prerelease) the newest prerelease, asked of GitHub's
# releases API.
#   anything else      passed on to dadoc bootstrap (e.g. --yes, --no-path)
#
# With sh reading the script from a pipe, pass options as: ... | sh -s -- --dir ~/tools/dado
set -eu

base=${DADO_RELEASE_BASE:-https://github.com/dadolangcom/Dado/releases}
dir="$HOME/dado"
version=""
rest=""
while [ $# -gt 0 ]; do
    case "$1" in
        --dir) dir=$2; shift 2 ;;
        --dir=*) dir=${1#--dir=}; shift ;;
        --version) version=$2; shift 2 ;;
        --version=*) version=${1#--version=}; shift ;;
        *) rest="$rest $1"; shift ;;
    esac
done

say() { printf 'dado install: %s\n' "$*" >&2; }
die() { say "$*"; exit 1; }

case "$(uname -s)" in
    Darwin) asset=dado-macos-universal.tar.gz ;;
    Linux)
        case "$(uname -m)" in
            x86_64|amd64) asset=dado-linux-x86_64.tar.gz ;;
            aarch64|arm64) asset=dado-linux-aarch64.tar.gz ;;
            *) die "there is no Dado release for Linux on $(uname -m)" ;;
        esac ;;
    *) die "this script is for macOS and Linux; on Windows run: irm https://raw.githubusercontent.com/dadolangcom/Dado/main/install.ps1 | iex" ;;
esac

if [ -n "$version" ]; then
    url="$base/download/v${version#v}"
else
    url="$base/latest/download"
fi

command -v curl >/dev/null 2>&1 || die "curl is needed to download the release"
command -v tar >/dev/null 2>&1 || die "tar is needed to unpack the release"
if [ -e "$dir/dadoc" ]; then
    die "$dir already holds a Dado install: run \`dado upgrade\` there, or pass --dir for another folder"
fi

# The newest release's tag, prereleases included: GitHub's API lists a
# repository's releases newest first, and drafts only to its owners. Answers
# nothing when the API cannot be reached or $base is not a GitHub repository.
newest_tag() {
    api=${DADO_RELEASE_API:-}
    if [ -z "$api" ]; then
        case "$base" in
            https://github.com/*/*/releases) repo=${base#https://github.com/}; api="https://api.github.com/repos/${repo%/releases}/releases" ;;
            *) return 0 ;;
        esac
    fi
    curl -fsSL -H 'Accept: application/vnd.github+json' "$api?per_page=1" 2>/dev/null | tr ',' '\n' |
        sed -n 's/.*"tag_name"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' | head -n 1
}

tmp=$(mktemp -d "${TMPDIR:-/tmp}/dado-install.XXXXXX")
trap 'rm -rf "$tmp"' EXIT INT TERM
say "downloading $asset"
if [ -n "$version" ]; then
    curl -fsSL "$url/$asset" -o "$tmp/$asset" || die "cannot download $url/$asset"
elif ! curl -fsSL "$url/$asset" -o "$tmp/$asset" 2>/dev/null; then
    # `latest` names the newest stable release and skips prereleases, so
    # before the first stable one it answers 404: take the newest prerelease.
    tag=$(newest_tag)
    [ -n "$tag" ] || die "cannot download $url/$asset, and found no prerelease to fall back to; pass a version from $base, for example: ... | sh -s -- --version 1.0.0-rc.3"
    url="$base/download/$tag"
    say "there is no stable release yet: downloading the newest prerelease, $tag"
    curl -fsSL "$url/$asset" -o "$tmp/$asset" || die "cannot download $url/$asset"
fi
curl -fsSL "$url/SHA256SUMS" -o "$tmp/SHA256SUMS" || die "cannot download $url/SHA256SUMS"
want=$(awk -v a="$asset" '$2 == a || $2 == "*" a { print $1 }' "$tmp/SHA256SUMS")
[ -n "$want" ] || die "SHA256SUMS has no line for $asset"
if command -v sha256sum >/dev/null 2>&1; then
    got=$(sha256sum "$tmp/$asset" | awk '{ print $1 }')
else
    got=$(shasum -a 256 "$tmp/$asset" | awk '{ print $1 }')
fi
[ "$got" = "$want" ] || die "$asset does not match SHA256SUMS (got $got, want $want); nothing was installed"

mkdir -p "$tmp/unpacked"
tar -xzf "$tmp/$asset" -C "$tmp/unpacked"
[ -x "$tmp/unpacked/dado/dadoc" ] || die "the archive holds no dado/dadoc"
mkdir -p "$dir"
cp -R "$tmp/unpacked/dado/." "$dir/"
say "unpacked into $dir"
# Read from a pipe, this script's standard input is the script itself, so the
# installer's questions are asked on the terminal when there is one.
# shellcheck disable=SC2086
if [ -t 1 ] && ( : < /dev/tty ) 2>/dev/null; then
    exec sh "$dir/install.sh" $rest < /dev/tty
fi
exec sh "$dir/install.sh" $rest
