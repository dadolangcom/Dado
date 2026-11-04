<!-- dadoc 1.0.0-rc.8.1 -->
<!-- commit 89a284bab714 -->
# vendor:raylib

vendor:raylib — a restatement of the whole of raylib 6.x's `raylib.h`.

Every struct, every enum, and every function but the three that take C's
varargs (below) — 655 declarations.

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

The 153 places the header says `const char *` say `cstring` here, which is
the type and not a spelling: a literal already is one, so the line above
needs nothing, and a `string` a program built — a score, a filename — reaches
them through `strings.clone_to_cstring(s)` and `cstring(that)`. There
is no conversion, and that allocation is what C's terminator costs. Where the
header says `char *` without the `const` — `LoadFileText`, `TextReplaceAlloc`
and the other functions answering text the caller owns and frees — it is
`^#c.char`, because that is a different C type and a different promise.

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

655 declarations, 655 public.

* `enum @c("ConfigFlags") config_flags`
* `enum @c("TraceLogLevel") trace_log_level`
* `enum @c("KeyboardKey") key`
* `enum @c("MouseButton") mouse_button`
* `enum @c("MouseCursor") mouse_cursor`
* `enum @c("GamepadButton") gamepad_button`
* `enum @c("GamepadAxis") gamepad_axis`
* `enum @c("MaterialMapIndex") material_map_index`
* `enum @c("ShaderLocationIndex") shader_location_index`
* `enum @c("ShaderUniformDataType") shader_uniform_data_type`
* `enum @c("ShaderAttributeDataType") shader_attribute_data_type`
* `enum @c("PixelFormat") pixel_format`
* `enum @c("TextureFilter") texture_filter`
* `enum @c("TextureWrap") texture_wrap`
* `enum @c("CubemapLayout") cubemap_layout`
* `enum @c("FontType") font_type`
* `enum @c("BlendMode") blend_mode`
* `enum @c("Gesture") gesture`
* `enum @c("CameraMode") camera_mode`
* `enum @c("CameraProjection") camera_projection`
* `enum @c("NPatchLayout") npatch_layout`
* `type Vector2`
* `type Vector3`
* `type Vector4`
* `type Matrix`
* `type Color`
* `type Rectangle`
* `type Image`
* `type Texture2D`
* `type RenderTexture2D`
* `type NPatchInfo`
* `type GlyphInfo`
* `type Font`
* `type Camera3D`
* `type Camera2D`
* `type Mesh`
* `type Shader`
* `type MaterialMap`
* `type Material`
* `type Transform`
* `type BoneInfo`
* `type ModelSkeleton`
* `type Model`
* `type ModelAnimation`
* `type Ray`
* `type RayCollision`
* `type BoundingBox`
* `type Wave`
* `type @incomplete rAudioBuffer`
* `type @incomplete rAudioProcessor`
* `type AudioStream`
* `type Sound`
* `type Music`
* `type VrDeviceInfo`
* `type VrStereoConfig`
* `type FilePathList`
* `type AutomationEvent`
* `type AutomationEventList`
* `void InitWindow(i32 width, i32 height, cstring title)`
* `void CloseWindow()`
* `bool WindowShouldClose()`
* `bool IsWindowReady()`
* `bool IsWindowFullscreen()`
* `bool IsWindowHidden()`
* `bool IsWindowMinimized()`
* `bool IsWindowMaximized()`
* `bool IsWindowFocused()`
* `bool IsWindowResized()`
* `bool IsWindowState(config_flags flag)`
* `void SetWindowState(config_flags flags)`
* `void ClearWindowState(config_flags flags)`
* `void ToggleFullscreen()`
* `void ToggleBorderlessWindowed()`
* `void MaximizeWindow()`
* `void MinimizeWindow()`
* `void RestoreWindow()`
* `void SetWindowIcon(Image image)`
* `void SetWindowIcons(^Image images, i32 count)`
* `void SetWindowTitle(cstring title)`
* `void SetWindowPosition(i32 x, i32 y)`
* `void SetWindowMonitor(i32 monitor)`
* `void SetWindowMinSize(i32 width, i32 height)`
* `void SetWindowMaxSize(i32 width, i32 height)`
* `void SetWindowSize(i32 width, i32 height)`
* `void SetWindowOpacity(f32 opacity)`
* `void SetWindowFocused()`
* `rawptr GetWindowHandle()`
* `i32 GetScreenWidth()`
* `i32 GetScreenHeight()`
* `i32 GetRenderWidth()`
* `i32 GetRenderHeight()`
* `i32 GetMonitorCount()`
* `i32 GetCurrentMonitor()`
* `Vector2 GetMonitorPosition(i32 monitor)`
* `i32 GetMonitorWidth(i32 monitor)`
* `i32 GetMonitorHeight(i32 monitor)`
* `i32 GetMonitorPhysicalWidth(i32 monitor)`
* `i32 GetMonitorPhysicalHeight(i32 monitor)`
* `i32 GetMonitorRefreshRate(i32 monitor)`
* `Vector2 GetWindowPosition()`
* `Vector2 GetWindowScaleDPI()`
* `cstring GetMonitorName(i32 monitor)`
* `void SetClipboardText(cstring text)`
* `cstring GetClipboardText()`
* `Image GetClipboardImage()`
* `void EnableEventWaiting()`
* `void DisableEventWaiting()`
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
* `void BeginMode3D(Camera3D camera)`
* `void EndMode3D()`
* `void BeginTextureMode(RenderTexture2D target)`
* `void EndTextureMode()`
* `void BeginShaderMode(Shader shader)`
* `void EndShaderMode()`
* `void BeginBlendMode(blend_mode mode)`
* `void EndBlendMode()`
* `void BeginScissorMode(i32 x, i32 y, i32 width, i32 height)`
* `void EndScissorMode()`
* `void BeginVrStereoMode(VrStereoConfig config)`
* `void EndVrStereoMode()`
* `VrStereoConfig LoadVrStereoConfig(VrDeviceInfo device)`
* `void UnloadVrStereoConfig(VrStereoConfig config)`
* `Shader LoadShader(cstring vsFileName, cstring fsFileName)`
* `Shader LoadShaderFromMemory(cstring vsCode, cstring fsCode)`
* `bool IsShaderValid(Shader shader)`
* `i32 GetShaderLocation(Shader shader, cstring uniformName)`
* `i32 GetShaderLocationAttrib(Shader shader, cstring attribName)`
* `void SetShaderValue(Shader shader, i32 locIndex, const rawptr value, shader_uniform_data_type uniformType)`
* `void SetShaderValueV(Shader shader, i32 locIndex, const rawptr value, shader_uniform_data_type uniformType, i32 count)`
* `void SetShaderValueMatrix(Shader shader, i32 locIndex, Matrix mat)`
* `void SetShaderValueTexture(Shader shader, i32 locIndex, Texture2D texture)`
* `void UnloadShader(Shader shader)`
* `Ray GetScreenToWorldRay(Vector2 position, Camera3D camera)`
* `Ray GetScreenToWorldRayEx(Vector2 position, Camera3D camera, i32 width, i32 height)`
* `Vector2 GetWorldToScreen(Vector3 position, Camera3D camera)`
* `Vector2 GetWorldToScreenEx(Vector3 position, Camera3D camera, i32 width, i32 height)`
* `Vector2 GetWorldToScreen2D(Vector2 position, Camera2D camera)`
* `Vector2 GetScreenToWorld2D(Vector2 position, Camera2D camera)`
* `Matrix GetCameraMatrix(Camera3D camera)`
* `Matrix GetCameraMatrix2D(Camera2D camera)`
* `void SetTargetFPS(i32 fps)`
* `f32 GetFrameTime()`
* `f64 GetTime()`
* `i32 GetFPS()`
* `void SwapScreenBuffer()`
* `void PollInputEvents()`
* `void WaitTime(f64 seconds)`
* `void SetRandomSeed(u32 seed)`
* `i32 GetRandomValue(i32 min, i32 max)`
* `^i32 LoadRandomSequence(u32 count, i32 min, i32 max)`
* `void UnloadRandomSequence(^i32 sequence)`
* `void TakeScreenshot(cstring fileName)`
* `void SetConfigFlags(config_flags flags)`
* `void OpenURL(cstring url)`
* `void SetTraceLogLevel(trace_log_level logLevel)`
* `rawptr MemAlloc(u32 size)`
* `rawptr MemRealloc(rawptr ptr, u32 size)`
* `void MemFree(rawptr ptr)`
* `^u8 LoadFileData(cstring fileName, ^i32 dataSize)`
* `void UnloadFileData(^u8 data)`
* `bool SaveFileData(cstring fileName, rawptr data, i32 dataSize)`
* `bool ExportDataAsCode(^const u8 data, i32 dataSize, cstring fileName)`
* `^#c.char LoadFileText(cstring fileName)`
* `void UnloadFileText(^#c.char text)`
* `bool SaveFileText(cstring fileName, cstring text)`
* `void SetLoadFileDataCallback((^u8)(cstring, ^i32) callback)`
* `void SetSaveFileDataCallback(bool(cstring, rawptr, i32) callback)`
* `void SetLoadFileTextCallback((^#c.char)(cstring) callback)`
* `void SetSaveFileTextCallback(bool(cstring, cstring) callback)`
* `i32 FileRename(cstring fileName, cstring fileRename)`
* `i32 FileRemove(cstring fileName)`
* `i32 FileCopy(cstring srcPath, cstring dstPath)`
* `i32 FileMove(cstring srcPath, cstring dstPath)`
* `i32 FileTextReplace(cstring fileName, cstring search, cstring replacement)`
* `i32 FileTextFindIndex(cstring fileName, cstring search)`
* `bool FileExists(cstring fileName)`
* `bool DirectoryExists(cstring dirPath)`
* `bool IsFileExtension(cstring fileName, cstring ext)`
* `i32 GetFileLength(cstring fileName)`
* `#c.long GetFileModTime(cstring fileName)`
* `cstring GetFileExtension(cstring fileName)`
* `cstring GetFileName(cstring filePath)`
* `cstring GetFileNameWithoutExt(cstring filePath)`
* `cstring GetDirectoryPath(cstring filePath)`
* `cstring GetPrevDirectoryPath(cstring dirPath)`
* `cstring GetWorkingDirectory()`
* `cstring GetApplicationDirectory()`
* `i32 MakeDirectory(cstring dirPath)`
* `bool ChangeDirectory(cstring dirPath)`
* `bool IsPathFile(cstring path)`
* `bool IsFileNameValid(cstring fileName)`
* `FilePathList LoadDirectoryFiles(cstring dirPath)`
* `FilePathList LoadDirectoryFilesEx(cstring basePath, cstring filter, bool scanSubdirs)`
* `void UnloadDirectoryFiles(FilePathList files)`
* `bool IsFileDropped()`
* `FilePathList LoadDroppedFiles()`
* `void UnloadDroppedFiles(FilePathList files)`
* `u32 GetDirectoryFileCount(cstring dirPath)`
* `u32 GetDirectoryFileCountEx(cstring basePath, cstring filter, bool scanSubdirs)`
* `^u8 CompressData(^const u8 data, i32 dataSize, ^i32 compDataSize)`
* `^u8 DecompressData(^const u8 compData, i32 compDataSize, ^i32 dataSize)`
* `^#c.char EncodeDataBase64(^const u8 data, i32 dataSize, ^i32 outputSize)`
* `^u8 DecodeDataBase64(cstring text, ^i32 outputSize)`
* `u32 ComputeCRC32(^u8 data, i32 dataSize)`
* `^u32 ComputeMD5(^u8 data, i32 dataSize)`
* `^u32 ComputeSHA1(^u8 data, i32 dataSize)`
* `^u32 ComputeSHA256(^u8 data, i32 dataSize)`
* `AutomationEventList LoadAutomationEventList(cstring fileName)`
* `void UnloadAutomationEventList(AutomationEventList list)`
* `bool ExportAutomationEventList(AutomationEventList list, cstring fileName)`
* `void SetAutomationEventList(^AutomationEventList list)`
* `void SetAutomationEventBaseFrame(i32 frame)`
* `void StartAutomationEventRecording()`
* `void StopAutomationEventRecording()`
* `void PlayAutomationEvent(AutomationEvent event)`
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
* `i32 SetGamepadMappings(cstring mappings)`
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
* `i32 GetTouchX()`
* `i32 GetTouchY()`
* `Vector2 GetTouchPosition(i32 index)`
* `i32 GetTouchPointId(i32 index)`
* `i32 GetTouchPointCount()`
* `void SetGesturesEnabled(gesture flags)`
* `bool IsGestureDetected(gesture gesture)`
* `i32 GetGestureDetected()`
* `f32 GetGestureHoldDuration()`
* `Vector2 GetGestureDragVector()`
* `f32 GetGestureDragAngle()`
* `Vector2 GetGesturePinchVector()`
* `f32 GetGesturePinchAngle()`
* `void UpdateCamera(^Camera3D camera, camera_mode mode)`
* `void UpdateCameraPro(^Camera3D camera, Vector3 movement, Vector3 rotation, f32 zoom)`
* `void SetShapesTexture(Texture2D texture, Rectangle source)`
* `Texture2D GetShapesTexture()`
* `Rectangle GetShapesTextureRectangle()`
* `void DrawPixel(i32 posX, i32 posY, Color color)`
* `void DrawPixelV(Vector2 position, Color color)`
* `void DrawLine(i32 startPosX, i32 startPosY, i32 endPosX, i32 endPosY, Color color)`
* `void DrawLineV(Vector2 startPos, Vector2 endPos, Color color)`
* `void DrawLineEx(Vector2 startPos, Vector2 endPos, f32 thick, Color color)`
* `void DrawLineStrip(^const Vector2 points, i32 pointCount, Color color)`
* `void DrawLineBezier(Vector2 startPos, Vector2 endPos, f32 thick, Color color)`
* `void DrawLineDashed(Vector2 startPos, Vector2 endPos, i32 dashSize, i32 spaceSize, Color color)`
* `void DrawCircle(i32 centerX, i32 centerY, f32 radius, Color color)`
* `void DrawCircleV(Vector2 center, f32 radius, Color color)`
* `void DrawCircleGradient(Vector2 center, f32 radius, Color inner, Color outer)`
* `void DrawCircleSector(Vector2 center, f32 radius, f32 startAngle, f32 endAngle, i32 segments, Color color)`
* `void DrawCircleSectorLines(Vector2 center, f32 radius, f32 startAngle, f32 endAngle, i32 segments, Color color)`
* `void DrawCircleLines(i32 centerX, i32 centerY, f32 radius, Color color)`
* `void DrawCircleLinesV(Vector2 center, f32 radius, Color color)`
* `void DrawEllipse(i32 centerX, i32 centerY, f32 radiusH, f32 radiusV, Color color)`
* `void DrawEllipseV(Vector2 center, f32 radiusH, f32 radiusV, Color color)`
* `void DrawEllipseLines(i32 centerX, i32 centerY, f32 radiusH, f32 radiusV, Color color)`
* `void DrawEllipseLinesV(Vector2 center, f32 radiusH, f32 radiusV, Color color)`
* `void DrawRing(Vector2 center, f32 innerRadius, f32 outerRadius, f32 startAngle, f32 endAngle, i32 segments, Color color)`
* `void DrawRingLines(Vector2 center, f32 innerRadius, f32 outerRadius, f32 startAngle, f32 endAngle, i32 segments, Color color)`
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
* `void DrawTriangleFan(^const Vector2 points, i32 pointCount, Color color)`
* `void DrawTriangleStrip(^const Vector2 points, i32 pointCount, Color color)`
* `void DrawPoly(Vector2 center, i32 sides, f32 radius, f32 rotation, Color color)`
* `void DrawPolyLines(Vector2 center, i32 sides, f32 radius, f32 rotation, Color color)`
* `void DrawPolyLinesEx(Vector2 center, i32 sides, f32 radius, f32 rotation, f32 lineThick, Color color)`
* `void DrawSplineLinear(^const Vector2 points, i32 pointCount, f32 thick, Color color)`
* `void DrawSplineBasis(^const Vector2 points, i32 pointCount, f32 thick, Color color)`
* `void DrawSplineCatmullRom(^const Vector2 points, i32 pointCount, f32 thick, Color color)`
* `void DrawSplineBezierQuadratic(^const Vector2 points, i32 pointCount, f32 thick, Color color)`
* `void DrawSplineBezierCubic(^const Vector2 points, i32 pointCount, f32 thick, Color color)`
* `void DrawSplineSegmentLinear(Vector2 p1, Vector2 p2, f32 thick, Color color)`
* `void DrawSplineSegmentBasis(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, f32 thick, Color color)`
* `void DrawSplineSegmentCatmullRom(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, f32 thick, Color color)`
* `void DrawSplineSegmentBezierQuadratic(Vector2 p1, Vector2 c2, Vector2 p3, f32 thick, Color color)`
* `void DrawSplineSegmentBezierCubic(Vector2 p1, Vector2 c2, Vector2 c3, Vector2 p4, f32 thick, Color color)`
* `Vector2 GetSplinePointLinear(Vector2 startPos, Vector2 endPos, f32 t)`
* `Vector2 GetSplinePointBasis(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, f32 t)`
* `Vector2 GetSplinePointCatmullRom(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, f32 t)`
* `Vector2 GetSplinePointBezierQuad(Vector2 p1, Vector2 c2, Vector2 p3, f32 t)`
* `Vector2 GetSplinePointBezierCubic(Vector2 p1, Vector2 c2, Vector2 c3, Vector2 p4, f32 t)`
* `bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2)`
* `bool CheckCollisionCircles(Vector2 center1, f32 radius1, Vector2 center2, f32 radius2)`
* `bool CheckCollisionCircleRec(Vector2 center, f32 radius, Rectangle rec)`
* `bool CheckCollisionCircleLine(Vector2 center, f32 radius, Vector2 p1, Vector2 p2)`
* `bool CheckCollisionPointRec(Vector2 point, Rectangle rec)`
* `bool CheckCollisionPointCircle(Vector2 point, Vector2 center, f32 radius)`
* `bool CheckCollisionPointTriangle(Vector2 point, Vector2 p1, Vector2 p2, Vector2 p3)`
* `bool CheckCollisionPointLine(Vector2 point, Vector2 p1, Vector2 p2, i32 threshold)`
* `bool CheckCollisionPointPoly(Vector2 point, ^const Vector2 points, i32 pointCount)`
* `bool CheckCollisionLines(Vector2 startPos1, Vector2 endPos1, Vector2 startPos2, Vector2 endPos2, ^Vector2 collisionPoint)`
* `Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2)`
* `Image LoadImage(cstring fileName)`
* `Image LoadImageRaw(cstring fileName, i32 width, i32 height, pixel_format format, i32 headerSize)`
* `Image LoadImageAnim(cstring fileName, ^i32 frames)`
* `Image LoadImageAnimFromMemory(cstring fileType, ^const u8 fileData, i32 dataSize, ^i32 frames)`
* `Image LoadImageFromMemory(cstring fileType, ^const u8 fileData, i32 dataSize)`
* `Image LoadImageFromTexture(Texture2D texture)`
* `Image LoadImageFromScreen()`
* `bool IsImageValid(Image image)`
* `void UnloadImage(Image image)`
* `bool ExportImage(Image image, cstring fileName)`
* `^u8 ExportImageToMemory(Image image, cstring fileType, ^i32 fileSize)`
* `bool ExportImageAsCode(Image image, cstring fileName)`
* `Image GenImageColor(i32 width, i32 height, Color color)`
* `Image GenImageGradientLinear(i32 width, i32 height, i32 direction, Color start, Color end)`
* `Image GenImageGradientRadial(i32 width, i32 height, f32 density, Color inner, Color outer)`
* `Image GenImageGradientSquare(i32 width, i32 height, f32 density, Color inner, Color outer)`
* `Image GenImageChecked(i32 width, i32 height, i32 checksX, i32 checksY, Color col1, Color col2)`
* `Image GenImageWhiteNoise(i32 width, i32 height, f32 factor)`
* `Image GenImagePerlinNoise(i32 width, i32 height, i32 offsetX, i32 offsetY, f32 scale)`
* `Image GenImageCellular(i32 width, i32 height, i32 tileSize)`
* `Image GenImageText(i32 width, i32 height, cstring text)`
* `Image ImageCopy(Image image)`
* `Image ImageFromImage(Image image, Rectangle rec)`
* `Image ImageFromChannel(Image image, i32 selectedChannel)`
* `Image ImageText(cstring text, i32 fontSize, Color color)`
* `Image ImageTextEx(Font font, cstring text, f32 fontSize, f32 spacing, Color tint)`
* `void ImageFormat(^Image image, pixel_format newFormat)`
* `void ImageToPOT(^Image image, Color fill)`
* `void ImageCrop(^Image image, Rectangle crop)`
* `void ImageAlphaCrop(^Image image, f32 threshold)`
* `void ImageAlphaClear(^Image image, Color color, f32 threshold)`
* `void ImageAlphaMask(^Image image, Image alphaMask)`
* `void ImageAlphaPremultiply(^Image image)`
* `void ImageBlurGaussian(^Image image, i32 blurSize)`
* `void ImageKernelConvolution(^Image image, ^const f32 kernel, i32 kernelSize)`
* `void ImageResize(^Image image, i32 newWidth, i32 newHeight)`
* `void ImageResizeNN(^Image image, i32 newWidth, i32 newHeight)`
* `void ImageResizeCanvas(^Image image, i32 newWidth, i32 newHeight, i32 offsetX, i32 offsetY, Color fill)`
* `void ImageMipmaps(^Image image)`
* `void ImageDither(^Image image, i32 rBpp, i32 gBpp, i32 bBpp, i32 aBpp)`
* `void ImageFlipVertical(^Image image)`
* `void ImageFlipHorizontal(^Image image)`
* `void ImageRotate(^Image image, i32 degrees)`
* `void ImageRotateCW(^Image image)`
* `void ImageRotateCCW(^Image image)`
* `void ImageColorTint(^Image image, Color color)`
* `void ImageColorInvert(^Image image)`
* `void ImageColorGrayscale(^Image image)`
* `void ImageColorContrast(^Image image, f32 contrast)`
* `void ImageColorBrightness(^Image image, i32 brightness)`
* `void ImageColorReplace(^Image image, Color color, Color replace)`
* `^Color LoadImageColors(Image image)`
* `^Color LoadImagePalette(Image image, i32 maxPaletteSize, ^i32 colorCount)`
* `void UnloadImageColors(^Color colors)`
* `void UnloadImagePalette(^Color colors)`
* `Rectangle GetImageAlphaBorder(Image image, f32 threshold)`
* `Color GetImageColor(Image image, i32 x, i32 y)`
* `void ImageClearBackground(^Image dst, Color color)`
* `void ImageDrawPixel(^Image dst, i32 posX, i32 posY, Color color)`
* `void ImageDrawPixelV(^Image dst, Vector2 position, Color color)`
* `void ImageDrawLine(^Image dst, i32 startPosX, i32 startPosY, i32 endPosX, i32 endPosY, Color color)`
* `void ImageDrawLineV(^Image dst, Vector2 start, Vector2 end, Color color)`
* `void ImageDrawLineEx(^Image dst, Vector2 start, Vector2 end, i32 thick, Color color)`
* `void ImageDrawCircle(^Image dst, i32 centerX, i32 centerY, i32 radius, Color color)`
* `void ImageDrawCircleV(^Image dst, Vector2 center, i32 radius, Color color)`
* `void ImageDrawCircleLines(^Image dst, i32 centerX, i32 centerY, i32 radius, Color color)`
* `void ImageDrawCircleLinesV(^Image dst, Vector2 center, i32 radius, Color color)`
* `void ImageDrawRectangle(^Image dst, i32 posX, i32 posY, i32 width, i32 height, Color color)`
* `void ImageDrawRectangleV(^Image dst, Vector2 position, Vector2 size, Color color)`
* `void ImageDrawRectangleRec(^Image dst, Rectangle rec, Color color)`
* `void ImageDrawRectangleLines(^Image dst, Rectangle rec, i32 thick, Color color)`
* `void ImageDrawTriangle(^Image dst, Vector2 v1, Vector2 v2, Vector2 v3, Color color)`
* `void ImageDrawTriangleEx(^Image dst, Vector2 v1, Vector2 v2, Vector2 v3, Color c1, Color c2, Color c3)`
* `void ImageDrawTriangleLines(^Image dst, Vector2 v1, Vector2 v2, Vector2 v3, Color color)`
* `void ImageDrawTriangleFan(^Image dst, ^const Vector2 points, i32 pointCount, Color color)`
* `void ImageDrawTriangleStrip(^Image dst, ^const Vector2 points, i32 pointCount, Color color)`
* `void ImageDraw(^Image dst, Image src, Rectangle srcRec, Rectangle dstRec, Color tint)`
* `void ImageDrawText(^Image dst, cstring text, i32 posX, i32 posY, i32 fontSize, Color color)`
* `void ImageDrawTextEx(^Image dst, Font font, cstring text, Vector2 position, f32 fontSize, f32 spacing, Color tint)`
* `Texture2D LoadTexture(cstring fileName)`
* `Texture2D LoadTextureFromImage(Image image)`
* `Texture2D LoadTextureCubemap(Image image, cubemap_layout layout)`
* `RenderTexture2D LoadRenderTexture(i32 width, i32 height)`
* `bool IsTextureValid(Texture2D texture)`
* `void UnloadTexture(Texture2D texture)`
* `bool IsRenderTextureValid(RenderTexture2D target)`
* `void UnloadRenderTexture(RenderTexture2D target)`
* `void UpdateTexture(Texture2D texture, const rawptr pixels)`
* `void UpdateTextureRec(Texture2D texture, Rectangle rec, const rawptr pixels)`
* `void GenTextureMipmaps(^Texture2D texture)`
* `void SetTextureFilter(Texture2D texture, texture_filter filter)`
* `void SetTextureWrap(Texture2D texture, texture_wrap wrap)`
* `void DrawTexture(Texture2D texture, i32 posX, i32 posY, Color tint)`
* `void DrawTextureV(Texture2D texture, Vector2 position, Color tint)`
* `void DrawTextureEx(Texture2D texture, Vector2 position, f32 rotation, f32 scale, Color tint)`
* `void DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position, Color tint)`
* `void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, f32 rotation, Color tint)`
* `void DrawTextureNPatch(Texture2D texture, NPatchInfo nPatchInfo, Rectangle dest, Vector2 origin, f32 rotation, Color tint)`
* `bool ColorIsEqual(Color col1, Color col2)`
* `Color Fade(Color color, f32 alpha)`
* `i32 ColorToInt(Color color)`
* `Vector4 ColorNormalize(Color color)`
* `Color ColorFromNormalized(Vector4 normalized)`
* `Vector3 ColorToHSV(Color color)`
* `Color ColorFromHSV(f32 hue, f32 saturation, f32 value)`
* `Color ColorTint(Color color, Color tint)`
* `Color ColorBrightness(Color color, f32 factor)`
* `Color ColorContrast(Color color, f32 contrast)`
* `Color ColorAlpha(Color color, f32 alpha)`
* `Color ColorAlphaBlend(Color dst, Color src, Color tint)`
* `Color ColorLerp(Color color1, Color color2, f32 factor)`
* `Color GetColor(u32 hexValue)`
* `Color GetPixelColor(rawptr srcPtr, pixel_format format)`
* `void SetPixelColor(rawptr dstPtr, Color color, pixel_format format)`
* `i32 GetPixelDataSize(i32 width, i32 height, pixel_format format)`
* `Font GetFontDefault()`
* `Font LoadFont(cstring fileName)`
* `Font LoadFontEx(cstring fileName, i32 fontSize, ^const i32 codepoints, i32 codepointCount)`
* `Font LoadFontFromImage(Image image, Color key, i32 firstChar)`
* `Font LoadFontFromMemory(cstring fileType, ^const u8 fileData, i32 dataSize, i32 fontSize, ^const i32 codepoints, i32 codepointCount)`
* `bool IsFontValid(Font font)`
* `^GlyphInfo LoadFontData(^const u8 fileData, i32 dataSize, i32 fontSize, ^const i32 codepoints, i32 codepointCount, font_type type_, ^i32 glyphCount)`
* `Image GenImageFontAtlas(^const GlyphInfo glyphs, ^^Rectangle glyphRecs, i32 glyphCount, i32 fontSize, i32 padding, i32 packMethod)`
* `void UnloadFontData(^GlyphInfo glyphs, i32 glyphCount)`
* `void UnloadFont(Font font)`
* `bool ExportFontAsCode(Font font, cstring fileName)`
* `void DrawFPS(i32 posX, i32 posY)`
* `void DrawText(cstring text, i32 posX, i32 posY, i32 fontSize, Color color)`
* `void DrawTextEx(Font font, cstring text, Vector2 position, f32 fontSize, f32 spacing, Color tint)`
* `void DrawTextPro(Font font, cstring text, Vector2 position, Vector2 origin, f32 rotation, f32 fontSize, f32 spacing, Color tint)`
* `void DrawTextCodepoint(Font font, i32 codepoint, Vector2 position, f32 fontSize, Color tint)`
* `void DrawTextCodepoints(Font font, ^const i32 codepoints, i32 codepointCount, Vector2 position, f32 fontSize, f32 spacing, Color tint)`
* `void SetTextLineSpacing(i32 spacing)`
* `i32 MeasureText(cstring text, i32 fontSize)`
* `Vector2 MeasureTextEx(Font font, cstring text, f32 fontSize, f32 spacing)`
* `Vector2 MeasureTextCodepoints(Font font, ^const i32 codepoints, i32 length, f32 fontSize, f32 spacing)`
* `i32 GetGlyphIndex(Font font, i32 codepoint)`
* `GlyphInfo GetGlyphInfo(Font font, i32 codepoint)`
* `Rectangle GetGlyphAtlasRec(Font font, i32 codepoint)`
* `^#c.char LoadUTF8(^const i32 codepoints, i32 length)`
* `void UnloadUTF8(^#c.char text)`
* `^i32 LoadCodepoints(cstring text, ^i32 count)`
* `void UnloadCodepoints(^i32 codepoints)`
* `i32 GetCodepointCount(cstring text)`
* `i32 GetCodepoint(cstring text, ^i32 codepointSize)`
* `i32 GetCodepointNext(cstring text, ^i32 codepointSize)`
* `i32 GetCodepointPrevious(cstring text, ^i32 codepointSize)`
* `cstring CodepointToUTF8(i32 codepoint, ^i32 utf8Size)`
* `^^#c.char LoadTextLines(cstring text, ^i32 count)`
* `void UnloadTextLines(^^#c.char text, i32 lineCount)`
* `i32 TextCopy(^#c.char dst, cstring src)`
* `bool TextIsEqual(cstring text1, cstring text2)`
* `u32 TextLength(cstring text)`
* `cstring TextSubtext(cstring text, i32 position, i32 length)`
* `cstring TextRemoveSpaces(cstring text)`
* `^#c.char GetTextBetween(cstring text, cstring begin, cstring end)`
* `^#c.char TextReplace(cstring text, cstring search, cstring replacement)`
* `^#c.char TextReplaceAlloc(cstring text, cstring search, cstring replacement)`
* `^#c.char TextReplaceBetween(cstring text, cstring begin, cstring end, cstring replacement)`
* `^#c.char TextReplaceBetweenAlloc(cstring text, cstring begin, cstring end, cstring replacement)`
* `^#c.char TextInsert(cstring text, cstring insert, i32 position)`
* `^#c.char TextInsertAlloc(cstring text, cstring insert, i32 position)`
* `^#c.char TextJoin(^^#c.char textList, i32 count, cstring delimiter)`
* `^^#c.char TextSplit(cstring text, #c.char delimiter, ^i32 count)`
* `void TextAppend(^#c.char text, cstring append, ^i32 position)`
* `i32 TextFindIndex(cstring text, cstring search)`
* `^#c.char TextToUpper(cstring text)`
* `^#c.char TextToLower(cstring text)`
* `^#c.char TextToPascal(cstring text)`
* `^#c.char TextToSnake(cstring text)`
* `^#c.char TextToCamel(cstring text)`
* `i32 TextToInteger(cstring text)`
* `f32 TextToFloat(cstring text)`
* `void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color)`
* `void DrawPoint3D(Vector3 position, Color color)`
* `void DrawCircle3D(Vector3 center, f32 radius, Vector3 rotationAxis, f32 rotationAngle, Color color)`
* `void DrawTriangle3D(Vector3 v1, Vector3 v2, Vector3 v3, Color color)`
* `void DrawTriangleStrip3D(^const Vector3 points, i32 pointCount, Color color)`
* `void DrawCube(Vector3 position, f32 width, f32 height, f32 length, Color color)`
* `void DrawCubeV(Vector3 position, Vector3 size, Color color)`
* `void DrawCubeWires(Vector3 position, f32 width, f32 height, f32 length, Color color)`
* `void DrawCubeWiresV(Vector3 position, Vector3 size, Color color)`
* `void DrawSphere(Vector3 centerPos, f32 radius, Color color)`
* `void DrawSphereEx(Vector3 centerPos, f32 radius, i32 rings, i32 slices, Color color)`
* `void DrawSphereWires(Vector3 centerPos, f32 radius, i32 rings, i32 slices, Color color)`
* `void DrawCylinder(Vector3 position, f32 radiusTop, f32 radiusBottom, f32 height, i32 slices, Color color)`
* `void DrawCylinderEx(Vector3 startPos, Vector3 endPos, f32 startRadius, f32 endRadius, i32 sides, Color color)`
* `void DrawCylinderWires(Vector3 position, f32 radiusTop, f32 radiusBottom, f32 height, i32 slices, Color color)`
* `void DrawCylinderWiresEx(Vector3 startPos, Vector3 endPos, f32 startRadius, f32 endRadius, i32 sides, Color color)`
* `void DrawCapsule(Vector3 startPos, Vector3 endPos, f32 radius, i32 slices, i32 rings, Color color)`
* `void DrawCapsuleWires(Vector3 startPos, Vector3 endPos, f32 radius, i32 slices, i32 rings, Color color)`
* `void DrawPlane(Vector3 centerPos, Vector2 size, Color color)`
* `void DrawRay(Ray ray, Color color)`
* `void DrawGrid(i32 slices, f32 spacing)`
* `Model LoadModel(cstring fileName)`
* `Model LoadModelFromMesh(Mesh mesh)`
* `bool IsModelValid(Model model)`
* `void UnloadModel(Model model)`
* `BoundingBox GetModelBoundingBox(Model model)`
* `void DrawModel(Model model, Vector3 position, f32 scale, Color tint)`
* `void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, f32 rotationAngle, Vector3 scale, Color tint)`
* `void DrawModelWires(Model model, Vector3 position, f32 scale, Color tint)`
* `void DrawModelWiresEx(Model model, Vector3 position, Vector3 rotationAxis, f32 rotationAngle, Vector3 scale, Color tint)`
* `void DrawBoundingBox(BoundingBox box, Color color)`
* `void DrawBillboard(Camera3D camera, Texture2D texture, Vector3 position, f32 scale, Color tint)`
* `void DrawBillboardRec(Camera3D camera, Texture2D texture, Rectangle source, Vector3 position, Vector2 size, Color tint)`
* `void DrawBillboardPro(Camera3D camera, Texture2D texture, Rectangle source, Vector3 position, Vector3 up, Vector2 size, Vector2 origin, f32 rotation, Color tint)`
* `void UploadMesh(^Mesh mesh, bool dynamic)`
* `void UpdateMeshBuffer(Mesh mesh, i32 index, const rawptr data, i32 dataSize, i32 offset)`
* `void UnloadMesh(Mesh mesh)`
* `void DrawMesh(Mesh mesh, Material material, Matrix transform)`
* `void DrawMeshInstanced(Mesh mesh, Material material, ^const Matrix transforms, i32 instances)`
* `BoundingBox GetMeshBoundingBox(Mesh mesh)`
* `void GenMeshTangents(^Mesh mesh)`
* `bool ExportMesh(Mesh mesh, cstring fileName)`
* `bool ExportMeshAsCode(Mesh mesh, cstring fileName)`
* `Mesh GenMeshPoly(i32 sides, f32 radius)`
* `Mesh GenMeshPlane(f32 width, f32 length, i32 resX, i32 resZ)`
* `Mesh GenMeshCube(f32 width, f32 height, f32 length)`
* `Mesh GenMeshSphere(f32 radius, i32 rings, i32 slices)`
* `Mesh GenMeshHemiSphere(f32 radius, i32 rings, i32 slices)`
* `Mesh GenMeshCylinder(f32 radius, f32 height, i32 slices)`
* `Mesh GenMeshCone(f32 radius, f32 height, i32 slices)`
* `Mesh GenMeshTorus(f32 radius, f32 size, i32 radSeg, i32 sides)`
* `Mesh GenMeshKnot(f32 radius, f32 size, i32 radSeg, i32 sides)`
* `Mesh GenMeshHeightmap(Image heightmap, Vector3 size)`
* `Mesh GenMeshCubicmap(Image cubicmap, Vector3 cubeSize)`
* `^Material LoadMaterials(cstring fileName, ^i32 materialCount)`
* `Material LoadMaterialDefault()`
* `bool IsMaterialValid(Material material)`
* `void UnloadMaterial(Material material)`
* `void SetMaterialTexture(^Material material, material_map_index mapType, Texture2D texture)`
* `void SetModelMeshMaterial(^Model model, i32 meshId, i32 materialId)`
* `^ModelAnimation LoadModelAnimations(cstring fileName, ^i32 animCount)`
* `void UpdateModelAnimation(Model model, ModelAnimation anim, f32 frame)`
* `void UpdateModelAnimationEx(Model model, ModelAnimation animA, f32 frameA, ModelAnimation animB, f32 frameB, f32 blend)`
* `void UnloadModelAnimations(^ModelAnimation animations, i32 animCount)`
* `bool IsModelAnimationValid(Model model, ModelAnimation anim)`
* `bool CheckCollisionSpheres(Vector3 center1, f32 radius1, Vector3 center2, f32 radius2)`
* `bool CheckCollisionBoxes(BoundingBox box1, BoundingBox box2)`
* `bool CheckCollisionBoxSphere(BoundingBox box, Vector3 center, f32 radius)`
* `RayCollision GetRayCollisionSphere(Ray ray, Vector3 center, f32 radius)`
* `RayCollision GetRayCollisionBox(Ray ray, BoundingBox box)`
* `RayCollision GetRayCollisionMesh(Ray ray, Mesh mesh, Matrix transform)`
* `RayCollision GetRayCollisionTriangle(Ray ray, Vector3 p1, Vector3 p2, Vector3 p3)`
* `RayCollision GetRayCollisionQuad(Ray ray, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4)`
* `void InitAudioDevice()`
* `void CloseAudioDevice()`
* `bool IsAudioDeviceReady()`
* `void SetMasterVolume(f32 volume)`
* `f32 GetMasterVolume()`
* `Wave LoadWave(cstring fileName)`
* `Wave LoadWaveFromMemory(cstring fileType, ^const u8 fileData, i32 dataSize)`
* `bool IsWaveValid(Wave wave)`
* `Sound LoadSound(cstring fileName)`
* `Sound LoadSoundFromWave(Wave wave)`
* `Sound LoadSoundAlias(Sound source)`
* `bool IsSoundValid(Sound sound)`
* `void UpdateSound(Sound sound, const rawptr data, i32 sampleCount)`
* `void UnloadWave(Wave wave)`
* `void UnloadSound(Sound sound)`
* `void UnloadSoundAlias(Sound alias)`
* `bool ExportWave(Wave wave, cstring fileName)`
* `bool ExportWaveAsCode(Wave wave, cstring fileName)`
* `void PlaySound(Sound sound)`
* `void StopSound(Sound sound)`
* `void PauseSound(Sound sound)`
* `void ResumeSound(Sound sound)`
* `bool IsSoundPlaying(Sound sound)`
* `void SetSoundVolume(Sound sound, f32 volume)`
* `void SetSoundPitch(Sound sound, f32 pitch)`
* `void SetSoundPan(Sound sound, f32 pan)`
* `Wave WaveCopy(Wave wave)`
* `void WaveCrop(^Wave wave, i32 initFrame, i32 finalFrame)`
* `void WaveFormat(^Wave wave, i32 sampleRate, i32 sampleSize, i32 channels)`
* `^f32 LoadWaveSamples(Wave wave)`
* `void UnloadWaveSamples(^f32 samples)`
* `Music LoadMusicStream(cstring fileName)`
* `Music LoadMusicStreamFromMemory(cstring fileType, ^const u8 data, i32 dataSize)`
* `bool IsMusicValid(Music music)`
* `void UnloadMusicStream(Music music)`
* `void PlayMusicStream(Music music)`
* `bool IsMusicStreamPlaying(Music music)`
* `void UpdateMusicStream(Music music)`
* `void StopMusicStream(Music music)`
* `void PauseMusicStream(Music music)`
* `void ResumeMusicStream(Music music)`
* `void SeekMusicStream(Music music, f32 position)`
* `void SetMusicVolume(Music music, f32 volume)`
* `void SetMusicPitch(Music music, f32 pitch)`
* `void SetMusicPan(Music music, f32 pan)`
* `f32 GetMusicTimeLength(Music music)`
* `f32 GetMusicTimePlayed(Music music)`
* `AudioStream LoadAudioStream(u32 sampleRate, u32 sampleSize, u32 channels)`
* `bool IsAudioStreamValid(AudioStream stream)`
* `void UnloadAudioStream(AudioStream stream)`
* `void UpdateAudioStream(AudioStream stream, const rawptr data, i32 frameCount)`
* `bool IsAudioStreamProcessed(AudioStream stream)`
* `void PlayAudioStream(AudioStream stream)`
* `void PauseAudioStream(AudioStream stream)`
* `void ResumeAudioStream(AudioStream stream)`
* `bool IsAudioStreamPlaying(AudioStream stream)`
* `void StopAudioStream(AudioStream stream)`
* `void SetAudioStreamVolume(AudioStream stream, f32 volume)`
* `void SetAudioStreamPitch(AudioStream stream, f32 pitch)`
* `void SetAudioStreamPan(AudioStream stream, f32 pan)`
* `void SetAudioStreamBufferSizeDefault(i32 size)`
* `void SetAudioStreamCallback(AudioStream stream, void(rawptr, u32) callback)`
* `void AttachAudioStreamProcessor(AudioStream stream, void(rawptr, u32) processor)`
* `void DetachAudioStreamProcessor(AudioStream stream, void(rawptr, u32) processor)`
* `void AttachAudioMixedProcessor(void(rawptr, u32) processor)`
* `void DetachAudioMixedProcessor(void(rawptr, u32) processor)`
