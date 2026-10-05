<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# vendor:raylib

vendor:raylib — a restatement of the parts of raylib 6.x a game reaches for.

Every declaration here is a **restatement**: Dado says what it
believes `raylib.h` contains, the emitted C `#include <raylib.h>`s the real
header, and the C compiler is what proves the belief. Nothing here parses a
header, and a wrong line — a misspelled name, a fourth parameter, a `f32`
where the header says `double`, an `i32` where it says `unsigned int` — is a
build error naming the declaration, not a runtime surprise: the emitted C
asserts each function's type against the header's with `_Generic`, and each
member's. **One thing is not caught: two parameters
of one type written in the wrong order.** A C function type has no parameter
names, so `DrawPixel(i32 posY, i32 posX, …)` is the header's own type.

    import "vendor:raylib"
    raylib.InitWindow(800, 450, "demo")

The nine places the header says `const char *` say `cstring` here, which is
the type and not a spelling: a literal already is one, so the line above
needs nothing, and a `string` a program built — a score, a filename — reaches
them through `strings.clone_to_cstring(s, alloc)` and `cstring(that)`. There
is no conversion, and that allocation is what C's terminator costs.

A `foreign` member that is not `private` is an ordinary declaration of this
package, so `raylib.InitWindow` is reachable from anything that
imports this file, and the call emits `InitWindow` — the C name, unmangled
There is no wrapper layer, and that is the point:
including the header buys every check a wrapper would, so a wrapper is
written only where it earns its call — a name Dado chose over C's, a truer
return type, or a check that belongs in exactly one place.

The header this restates is vendored beside it, so the C compiles everywhere
and every restatement is checked everywhere — but linking still needs the
real library. `brew install raylib` on macOS; the distro's `-dev` package on
Linux. The `link` clauses below name `-lraylib` and the platform libraries.

## Declarations

163 declarations, 163 public.

* `enum @c("KeyboardKey") key` — bindgen:begin declarations — regenerated; edits between the markers are lost…
* `enum @c("MouseButton") mouse_button`
* `enum @c("MouseCursor") mouse_cursor`
* `enum @c("GamepadButton") gamepad_button`
* `enum @c("GamepadAxis") gamepad_axis`
* `enum @c("TextureFilter") texture_filter`
* `enum @c("TextureWrap") texture_wrap`
* `type Vector2` — ── Types ─────────────────────────────────────────────────────────────…
* `type Vector3`
* `type Vector4`
* `type Color`
* `type Rectangle`
* `type Texture2D`
* `type RenderTexture2D`
* `type Camera2D`
* `void InitWindow(i32 width, i32 height, cstring title)` — ── Functions ─────────────────────────────────────────────────────────…
* `void CloseWindow()`
* `bool WindowShouldClose()`
* `bool IsWindowReady()`
* `bool IsWindowFullscreen()`
* `bool IsWindowHidden()`
* `bool IsWindowMinimized()`
* `bool IsWindowMaximized()`
* `bool IsWindowFocused()`
* `bool IsWindowResized()`
* `void ToggleFullscreen()`
* `void ToggleBorderlessWindowed()`
* `void MaximizeWindow()`
* `void MinimizeWindow()`
* `void RestoreWindow()`
* `void SetWindowTitle(cstring title)`
* `void SetWindowPosition(i32 x, i32 y)`
* `void SetWindowMinSize(i32 width, i32 height)`
* `void SetWindowMaxSize(i32 width, i32 height)`
* `void SetWindowSize(i32 width, i32 height)`
* `void SetWindowOpacity(f32 opacity)`
* `i32 GetScreenWidth()`
* `i32 GetScreenHeight()`
* `i32 GetRenderWidth()`
* `i32 GetRenderHeight()`
* `Vector2 GetWindowPosition()`
* `Vector2 GetWindowScaleDPI()`
* `void SetClipboardText(cstring text)`
* `cstring GetClipboardText()`
* `void ShowCursor()`
* `void HideCursor()`
* `bool IsCursorHidden()`
* `void EnableCursor()`
* `void DisableCursor()`
* `bool IsCursorOnScreen()`
* `void ClearBackground(Color color)`
* `void BeginDrawing()`
* `void EndDrawing()`
* `void BeginMode2D(Camera2D camera)`
* `void EndMode2D()`
* `void BeginScissorMode(i32 x, i32 y, i32 width, i32 height)`
* `void EndScissorMode()`
* `void SetTargetFPS(i32 fps)`
* `f32 GetFrameTime()`
* `f64 GetTime()`
* `i32 GetFPS()`
* `void SetRandomSeed(u32 seed)`
* `i32 GetRandomValue(i32 min, i32 max)`
* `bool IsKeyPressed(key key)`
* `bool IsKeyPressedRepeat(key key)`
* `bool IsKeyDown(key key)`
* `bool IsKeyReleased(key key)`
* `bool IsKeyUp(key key)`
* `i32 GetKeyPressed()`
* `i32 GetCharPressed()`
* `cstring GetKeyName(key key)`
* `void SetExitKey(key key)`
* `bool IsGamepadAvailable(i32 gamepad)`
* `cstring GetGamepadName(i32 gamepad)`
* `bool IsGamepadButtonPressed(i32 gamepad, gamepad_button button)`
* `bool IsGamepadButtonDown(i32 gamepad, gamepad_button button)`
* `bool IsGamepadButtonReleased(i32 gamepad, gamepad_button button)`
* `bool IsGamepadButtonUp(i32 gamepad, gamepad_button button)`
* `i32 GetGamepadButtonPressed()`
* `i32 GetGamepadAxisCount(i32 gamepad)`
* `f32 GetGamepadAxisMovement(i32 gamepad, gamepad_axis axis)`
* `void SetGamepadVibration(i32 gamepad, f32 leftMotor, f32 rightMotor, f32 duration)`
* `bool IsMouseButtonPressed(mouse_button button)`
* `bool IsMouseButtonDown(mouse_button button)`
* `bool IsMouseButtonReleased(mouse_button button)`
* `bool IsMouseButtonUp(mouse_button button)`
* `i32 GetMouseX()`
* `i32 GetMouseY()`
* `Vector2 GetMousePosition()`
* `Vector2 GetMouseDelta()`
* `void SetMousePosition(i32 x, i32 y)`
* `void SetMouseOffset(i32 offsetX, i32 offsetY)`
* `void SetMouseScale(f32 scaleX, f32 scaleY)`
* `f32 GetMouseWheelMove()`
* `Vector2 GetMouseWheelMoveV()`
* `void SetMouseCursor(mouse_cursor cursor)`
* `void DrawPixel(i32 posX, i32 posY, Color color)`
* `void DrawPixelV(Vector2 position, Color color)`
* `void DrawLine(i32 startPosX, i32 startPosY, i32 endPosX, i32 endPosY, Color color)`
* `void DrawLineV(Vector2 startPos, Vector2 endPos, Color color)`
* `void DrawLineEx(Vector2 startPos, Vector2 endPos, f32 thick, Color color)`
* `void DrawLineBezier(Vector2 startPos, Vector2 endPos, f32 thick, Color color)`
* `void DrawLineDashed(Vector2 startPos, Vector2 endPos, i32 dashSize, i32 spaceSize, Color color)`
* `void DrawCircle(i32 centerX, i32 centerY, f32 radius, Color color)`
* `void DrawCircleV(Vector2 center, f32 radius, Color color)`
* `void DrawCircleLines(i32 centerX, i32 centerY, f32 radius, Color color)`
* `void DrawCircleLinesV(Vector2 center, f32 radius, Color color)`
* `void DrawEllipse(i32 centerX, i32 centerY, f32 radiusH, f32 radiusV, Color color)`
* `void DrawEllipseV(Vector2 center, f32 radiusH, f32 radiusV, Color color)`
* `void DrawEllipseLines(i32 centerX, i32 centerY, f32 radiusH, f32 radiusV, Color color)`
* `void DrawEllipseLinesV(Vector2 center, f32 radiusH, f32 radiusV, Color color)`
* `void DrawRectangle(i32 posX, i32 posY, i32 width, i32 height, Color color)`
* `void DrawRectangleV(Vector2 position, Vector2 size, Color color)`
* `void DrawRectangleRec(Rectangle rec, Color color)`
* `void DrawRectanglePro(Rectangle rec, Vector2 origin, f32 rotation, Color color)`
* `void DrawRectangleGradientV(i32 posX, i32 posY, i32 width, i32 height, Color top, Color bottom)`
* `void DrawRectangleGradientH(i32 posX, i32 posY, i32 width, i32 height, Color left, Color right)`
* `void DrawRectangleGradientEx(Rectangle rec, Color topLeft, Color bottomLeft, Color bottomRight, Color topRight)`
* `void DrawRectangleLines(i32 posX, i32 posY, i32 width, i32 height, Color color)`
* `void DrawRectangleLinesEx(Rectangle rec, f32 lineThick, Color color)`
* `void DrawRectangleRounded(Rectangle rec, f32 roundness, i32 segments, Color color)`
* `void DrawRectangleRoundedLines(Rectangle rec, f32 roundness, i32 segments, Color color)`
* `void DrawRectangleRoundedLinesEx(Rectangle rec, f32 roundness, i32 segments, f32 lineThick, Color color)`
* `void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color)`
* `void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color)`
* `void DrawPoly(Vector2 center, i32 sides, f32 radius, f32 rotation, Color color)`
* `void DrawPolyLines(Vector2 center, i32 sides, f32 radius, f32 rotation, Color color)`
* `void DrawPolyLinesEx(Vector2 center, i32 sides, f32 radius, f32 rotation, f32 lineThick, Color color)`
* `bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2)`
* `bool CheckCollisionCircles(Vector2 center1, f32 radius1, Vector2 center2, f32 radius2)`
* `bool CheckCollisionCircleRec(Vector2 center, f32 radius, Rectangle rec)`
* `bool CheckCollisionCircleLine(Vector2 center, f32 radius, Vector2 p1, Vector2 p2)`
* `bool CheckCollisionPointRec(Vector2 point, Rectangle rec)`
* `bool CheckCollisionPointCircle(Vector2 point, Vector2 center, f32 radius)`
* `bool CheckCollisionPointTriangle(Vector2 point, Vector2 p1, Vector2 p2, Vector2 p3)`
* `bool CheckCollisionPointLine(Vector2 point, Vector2 p1, Vector2 p2, i32 threshold)`
* `Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2)`
* `Texture2D LoadTexture(cstring fileName)`
* `RenderTexture2D LoadRenderTexture(i32 width, i32 height)`
* `bool IsTextureValid(Texture2D texture)`
* `void UnloadTexture(Texture2D texture)`
* `bool IsRenderTextureValid(RenderTexture2D target)`
* `void UnloadRenderTexture(RenderTexture2D target)`
* `void SetTextureFilter(Texture2D texture, texture_filter filter)`
* `void SetTextureWrap(Texture2D texture, texture_wrap wrap)`
* `void DrawTexture(Texture2D texture, i32 posX, i32 posY, Color tint)`
* `void DrawTextureV(Texture2D texture, Vector2 position, Color tint)`
* `void DrawTextureEx(Texture2D texture, Vector2 position, f32 rotation, f32 scale, Color tint)`
* `void DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position, Color tint)`
* `void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, f32 rotation, Color tint)`
* `bool ColorIsEqual(Color col1, Color col2)`
* `Color Fade(Color color, f32 alpha)`
* `i32 ColorToInt(Color color)`
* `Color ColorFromHSV(f32 hue, f32 saturation, f32 value)`
* `Color ColorTint(Color color, Color tint)`
* `Color ColorBrightness(Color color, f32 factor)`
* `Color ColorContrast(Color color, f32 contrast)`
* `Color ColorAlpha(Color color, f32 alpha)`
* `Color ColorLerp(Color color1, Color color2, f32 factor)`
* `Color GetColor(u32 hexValue)`
* `void DrawFPS(i32 posX, i32 posY)`
* `void DrawText(cstring text, i32 posX, i32 posY, i32 fontSize, Color color)`
* `i32 MeasureText(cstring text, i32 fontSize)`
