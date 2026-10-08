<!-- dadoc 1.0.0-rc.3 -->
<!-- commit 8e62c891753a -->
# vendor:three

DadoScript classes over three.js r184, the JavaScript 3D library.

Each class wraps one three.js class and holds its object in `any h`. Import
the package from a `.dados` file and run that file in a script VM:

    import three "vendor:three"

    three.Scene scene = three.Scene()
    three.Mesh box = three.Mesh(three.BoxGeometry(1.0, 1.0, 1.0),
                                three.MeshStandardMaterial({"roughness": 0.5}))
    scene.add(box)

Targets. three.js is JavaScript, so the classes run wherever a script VM
runs: natively in the QuickJS engine Dado links, and on `#WEB` in the
browser's own JavaScript. The math and the scene graph (vectors, colours,
matrices, objects, geometry, materials, lights and cameras) work on both.
`WebGLRenderer` draws to a canvas through WebGL, so it works only in a
`#WEB` build loaded by a browser; natively, or under node, its constructor
fails with `document is not defined`. A `#NONE` build has no script VM
(`#script_vm` is ERR1205 there), so it can call the constants in
`three_consts.dado` and nothing else.

The `.dados` classes, `three_consts.dado` and `three_rt.js` are generated
from @types/three 0.184.1 with the settings in `dtsgen.toml`. Text between a
`dtsgen:begin` and a `dtsgen:end` line is regenerated, so it is never edited
by hand. `lib/` is three 0.184.0 from npm, unmodified (MIT, `lib/LICENSE`),
and `THREE_PIN` records its version and hashes.

## Declarations

913 declarations, 892 public.

* `any dts_make(any self, string name, ..any args)`
* `void dts_skip()`
* `void init(float width = 1.0, float height = 1.0, float depth = 1.0, int widthSegments = 1, int heightSegments = 1, int depthSegments = 1)`
* `any parameters()`
* `any dts_make(any self, string name, ..any args)`
* `void dts_adopt(any o, string name)`
* `any dts_cached(any o, string name)`
* `bool dts_nil(any v)`
* `any dts_u(any v)`
* `any dts_cb(any target, string method)`
* `any h = 0`
* `void init(any array, float itemSize, bool normalized = false)`
* `float id()`
* `string name()`
* `void set_name(string v)`
* `any array()`
* `void set_array(any v)`
* `float itemSize()`
* `void set_itemSize(float v)`
* `int usage()`
* `void set_usage(int v)`
* `int gpuType()`
* `void set_gpuType(int v)`
* `[]any updateRanges()`
* `void set_updateRanges([]any v)`
* `float version()`
* `void set_version(float v)`
* `bool normalized()`
* `void set_normalized(bool v)`
* `float count()`
* `bool needsUpdate()`
* `void set_needsUpdate(bool v)`
* `bool isBufferAttribute()`
* `BufferAttribute onUpload(any callback_target, string callback_method)`
* `BufferAttribute setUsage(int usage)`
* `void addUpdateRange(float start, float count)`
* `void clearUpdateRanges()`
* `BufferAttribute clone()`
* `BufferAttribute copy(BufferAttribute source)`
* `BufferAttribute copyAt(float index1, BufferAttribute attribute, float index2)`
* `BufferAttribute copyArray(any array)`
* `BufferAttribute applyMatrix4(Matrix4 m)`
* `BufferAttribute transformDirection(Matrix4 m)`
* `BufferAttribute set(any value, float offset = 0.0)`
* `float getComponent(float index, float component)`
* `void setComponent(float index, float component, float value)`
* `float getX(float index)`
* `BufferAttribute setX(float index, float x)`
* `float getY(float index)`
* `BufferAttribute setY(float index, float y)`
* `float getZ(float index)`
* `BufferAttribute setZ(float index, float z)`
* `float getW(float index)`
* `BufferAttribute setW(float index, float z)`
* `BufferAttribute setXY(float index, float x, float y)`
* `BufferAttribute setXYZ(float index, float x, float y, float z)`
* `BufferAttribute setXYZW(float index, float x, float y, float z, float w)`
* `any toJSON()`
* `void dispose()`
* `any dts_make(any self, string name, ..any args)`
* `void dts_adopt(any o, string name)`
* `any dts_cached(any o, string name)`
* `bool dts_nil(any v)`
* `any dts_u(any v)`
* `any dts_null()`
* `any h = 0`
* `void init()`
* `int id()`
* `void set_id(int v)`
* `string uuid()`
* `void set_uuid(string v)`
* `string name()`
* `void set_name(string v)`
* `string type_()`
* `bool has_index()`
* `BufferAttribute index()`
* `void set_index(BufferAttribute v)`
* `void clear_index()`
* `bool has_indirect()`
* `any indirect()`
* `void set_indirect(any v)`
* `void clear_indirect()`
* `any indirectOffset()`
* `void set_indirectOffset(any v)`
* `any attributes()`
* `void set_attributes(any v)`
* `any morphAttributes()`
* `void set_morphAttributes(any v)`
* `bool morphTargetsRelative()`
* `void set_morphTargetsRelative(bool v)`
* `[]any groups()`
* `void set_groups([]any v)`
* `any drawRange()`
* `void set_drawRange(any v)`
* `any userData()`
* `void set_userData(any v)`
* `bool isBufferGeometry()`
* `BufferGeometry setIndex(any index)`
* `BufferGeometry setIndirect(any indirect, any indirectOffset)`
* `any getIndirect()`
* `void addGroup(float start, float count, float materialIndex)`
* `void clearGroups()`
* `void setDrawRange(float start, float count)`
* `BufferGeometry applyMatrix4(Matrix4 matrix)`
* `BufferGeometry rotateX(float angle)`
* `BufferGeometry rotateY(float angle)`
* `BufferGeometry rotateZ(float angle)`
* `BufferGeometry translate(float x, float y, float z)`
* `BufferGeometry scale(float x, float y, float z)`
* `BufferGeometry lookAt(Vector3 vector)`
* `BufferGeometry center()`
* `BufferGeometry setFromPoints(any points)`
* `void computeBoundingBox()`
* `void computeBoundingSphere()`
* `void computeTangents()`
* `void computeVertexNormals()`
* `void normalizeNormals()`
* `BufferGeometry toNonIndexed()`
* `any toJSON()`
* `BufferGeometry clone()`
* `BufferGeometry copy(BufferGeometry source)`
* `void dispose()`
* `any dts_make(any self, string name, ..any args)`
* `void dts_skip()`
* `void init()`
* `bool isCamera()`
* `Matrix4 matrixWorldInverse()`
* `void set_matrixWorldInverse(Matrix4 v)`
* `Matrix4 projectionMatrix()`
* `void set_projectionMatrix(Matrix4 v)`
* `Matrix4 projectionMatrixInverse()`
* `void set_projectionMatrixInverse(Matrix4 v)`
* `int coordinateSystem()`
* `void set_coordinateSystem(int v)`
* `bool reversedDepth()`
* `any dts_make(any self, string name, ..any args)`
* `void dts_adopt(any o, string name)`
* `any dts_cached(any o, string name)`
* `bool dts_nil(any v)`
* `any dts_u(any v)`
* `any h = 0`
* `void init(float r, float g, float b)`
* `bool isColor()`
* `float r()`
* `void set_r(float v)`
* `float g()`
* `void set_g(float v)`
* `float b()`
* `void set_b(float v)`
* `Color setFromVector3(Vector3 vector)`
* `Color setScalar(float scalar)`
* `Color setHex(int hex)`
* `Color setRGB(float r, float g, float b, string colorSpace)`
* `Color setHSL(float h_, float s, float l, string colorSpace)`
* `Color setStyle(string style, string colorSpace)`
* `Color setColorName(string style, string colorSpace)`
* `Color clone()`
* `Color copy(Color color)`
* `Color copySRGBToLinear(Color color)`
* `Color copyLinearToSRGB(Color color)`
* `Color convertSRGBToLinear()`
* `Color convertLinearToSRGB()`
* `int getHex()`
* `string getHexString()`
* `any getHSL(any target, string colorSpace)`
* `any getRGB(any target, string colorSpace)`
* `string getStyle()`
* `Color offsetHSL(float h_, float s, float l)`
* `Color add(Color color)`
* `Color addColors(Color color1, Color color2)`
* `Color addScalar(float s)`
* `Color sub(Color color)`
* `Color multiply(Color color)`
* `Color multiplyScalar(float s)`
* `Color lerp(Color color, float alpha)`
* `Color lerpColors(Color color1, Color color2, float alpha)`
* `Color lerpHSL(Color color, float alpha)`
* `bool equals(Color color)`
* `Color fromArray(any array, float offset)`
* `[]float toArray([]float array, float offset)`
* `float toJSON()`
* `Color fromBufferAttribute(any attribute, float index)`
* `any dts_make(any self, string name, ..any args)`
* `void dts_skip()`
* `any dts_u(any v)`
* `void init(any color = 16777215, float intensity = 1.0)`
* `bool isDirectionalLight()`
* `Object3D target()`
* `void set_target(Object3D v)`
* `any shadow()`
* `void set_shadow(any v)`
* `any dts_make(any self, string name, ..any args)`
* `void dts_adopt(any o, string name)`
* `any dts_cached(any o, string name)`
* `bool dts_nil(any v)`
* `any dts_u(any v)`
* `any dts_cb(any target, string method)`
* `any h = 0`
* `void init(float x, float y, float z, string order)`
* `float x()`
* `void set_x(float v)`
* `float y()`
* `void set_y(float v)`
* `float z()`
* `void set_z(float v)`
* `string order()`
* `void set_order(string v)`
* `bool isEuler()`
* `Euler set(float x, float y, float z, string order)`
* `Euler clone()`
* `Euler copy(Euler euler)`
* `Euler setFromRotationMatrix(Matrix4 m, string order, bool update)`
* `Euler setFromVector3(Vector3 v, string order)`
* `Euler reorder(string newOrder)`
* `bool equals(Euler euler)`
* `Euler fromArray(any array)`
* `any toArray(any array, float offset)`
* `Euler _onChange(any callback_target, string callback_method)`
* `any dts_make(any self, string name, ..any args)`
* `void dts_skip()`
* `void dts_adopt(any o, string name)`
* `any dts_cached(any o, string name)`
* `bool dts_nil(any v)`
* `any dts_u(any v)`
* `any dts_null()`
* `void init(float size = 10.0, int divisions = 10, any color1 = 4473924, any color2 = 8947848)`
* `void dispose()`
* `bool isLineSegments()`
* `bool isLine()`
* `BufferGeometry geometry()`
* `void set_geometry(BufferGeometry v)`
* `bool has_morphTargetInfluences()`
* `[]float morphTargetInfluences()`
* `void set_morphTargetInfluences([]float v)`
* `void clear_morphTargetInfluences()`
* `bool has_morphTargetDictionary()`
* `any morphTargetDictionary()`
* `void set_morphTargetDictionary(any v)`
* `void clear_morphTargetDictionary()`
* `GridHelper computeLineDistances()`
* `void updateMorphTargets()`
* `any dts_make(any self, string name, ..any args)`
* `void dts_skip()`
* `any dts_u(any v)`
* `void init(any skyColor = 16777215, any groundColor = 16777215, float intensity = 1.0)`
* `bool isHemisphereLight()`
* `Color groundColor()`
* `void set_groundColor(Color v)`
* `any dts_make(any self, string name, ..any args)`
* `void dts_skip()`
* `void dts_adopt(any o, string name)`
* `any dts_cached(any o, string name)`
* `bool dts_nil(any v)`
* `any dts_u(any v)`
* `void init(any color = 16777215, float intensity = 1.0)`
* `bool isLight()`
* `Color color()`
* `void set_color(Color v)`
* `float intensity()`
* `void set_intensity(float v)`
* `void dispose()`
* `any dts_make(any self, string name, ..any args)`
* `void dts_adopt(any o, string name)`
* `any dts_cached(any o, string name)`
* `bool dts_nil(any v)`
* `any dts_u(any v)`
* `any dts_obj({string: any} m)`
* `any dts_null()`
* `any h = 0`
* `void init()`
* `string name()`
* `void set_name(string v)`
* `int blending()`
* `void set_blending(int v)`
* `int side()`
* `void set_side(int v)`
* `bool vertexColors()`
* `void set_vertexColors(bool v)`
* `float opacity()`
* `void set_opacity(float v)`
* `bool transparent()`
* `void set_transparent(bool v)`
* `bool alphaHash()`
* `void set_alphaHash(bool v)`
* `int blendSrc()`
* `void set_blendSrc(int v)`
* `int blendDst()`
* `void set_blendDst(int v)`
* `int blendEquation()`
* `void set_blendEquation(int v)`
* `bool has_blendSrcAlpha()`
* `int blendSrcAlpha()`
* `void set_blendSrcAlpha(int v)`
* `void clear_blendSrcAlpha()`
* `bool has_blendDstAlpha()`
* `int blendDstAlpha()`
* `void set_blendDstAlpha(int v)`
* `void clear_blendDstAlpha()`
* `bool has_blendEquationAlpha()`
* `int blendEquationAlpha()`
* `void set_blendEquationAlpha(int v)`
* `void clear_blendEquationAlpha()`
* `Color blendColor()`
* `void set_blendColor(Color v)`
* `float blendAlpha()`
* `void set_blendAlpha(float v)`
* `int depthFunc()`
* `void set_depthFunc(int v)`
* `bool depthTest()`
* `void set_depthTest(bool v)`
* `bool depthWrite()`
* `void set_depthWrite(bool v)`
* `float stencilWriteMask()`
* `void set_stencilWriteMask(float v)`
* `int stencilFunc()`
* `void set_stencilFunc(int v)`
* `float stencilRef()`
* `void set_stencilRef(float v)`
* `float stencilFuncMask()`
* `void set_stencilFuncMask(float v)`
* `int stencilFail()`
* `void set_stencilFail(int v)`
* `int stencilZFail()`
* `void set_stencilZFail(int v)`
* `int stencilZPass()`
* `void set_stencilZPass(int v)`
* `bool stencilWrite()`
* `void set_stencilWrite(bool v)`
* `bool clipIntersection()`
* `void set_clipIntersection(bool v)`
* `bool clipShadows()`
* `void set_clipShadows(bool v)`
* `bool has_shadowSide()`
* `int shadowSide()`
* `void set_shadowSide(int v)`
* `void clear_shadowSide()`
* `bool colorWrite()`
* `void set_colorWrite(bool v)`
* `bool has_precision()`
* `string precision()`
* `void set_precision(string v)`
* `void clear_precision()`
* `bool polygonOffset()`
* `void set_polygonOffset(bool v)`
* `float polygonOffsetFactor()`
* `void set_polygonOffsetFactor(float v)`
* `float polygonOffsetUnits()`
* `void set_polygonOffsetUnits(float v)`
* `bool dithering()`
* `void set_dithering(bool v)`
* `bool alphaToCoverage()`
* `void set_alphaToCoverage(bool v)`
* `bool premultipliedAlpha()`
* `void set_premultipliedAlpha(bool v)`
* `bool forceSinglePass()`
* `void set_forceSinglePass(bool v)`
* `bool allowOverride()`
* `void set_allowOverride(bool v)`
* `bool visible()`
* `void set_visible(bool v)`
* `bool toneMapped()`
* `void set_toneMapped(bool v)`
* `any userData()`
* `void set_userData(any v)`
* `float alphaTest()`
* `void set_alphaTest(float v)`
* `bool isMaterial()`
* `string uuid()`
* `string type_()`
* `void set_type(string v)`
* `float version()`
* `bool has_defines()`
* `any defines()`
* `void set_defines(any v)`
* `void clear_defines()`
* `void onBeforeCompile(any parameters, WebGLRenderer renderer)`
* `string customProgramCacheKey()`
* `void setValues({string: any} values)`
* `any toJSON(any meta)`
* `Material clone()`
* `Material copy(Material source)`
* `void dispose()`
* `bool needsUpdate()`
* `void set_needsUpdate(bool v)`
* `any dts_make(any self, string name, ..any args)`
* `void dts_adopt(any o, string name)`
* `any dts_cached(any o, string name)`
* `bool dts_nil(any v)`
* `any dts_u(any v)`
* `any h = 0`
* `void init()`
* `bool isMatrix4()`
* `[]float elements()`
* `void set_elements([]float v)`
* `Matrix4 set(float n11, float n12, float n13, float n14, float n21, float n22, float n23, float n24, float n31, float n32, float n33, float n34, float n41, float n42, float n43, float n44)`
* `Matrix4 identity()`
* `Matrix4 clone()`
* `Matrix4 copy(Matrix4 m)`
* `Matrix4 copyPosition(Matrix4 m)`
* `Matrix4 extractBasis(Vector3 xAxis, Vector3 yAxis, Vector3 zAxis)`
* `Matrix4 makeBasis(Vector3 xAxis, Vector3 yAxis, Vector3 zAxis)`
* `Matrix4 extractRotation(Matrix4 m)`
* `Matrix4 makeRotationFromEuler(Euler euler)`
* `Matrix4 lookAt(Vector3 eye, Vector3 target, Vector3 up)`
* `Matrix4 multiply(Matrix4 m)`
* `Matrix4 premultiply(Matrix4 m)`
* `Matrix4 multiplyMatrices(Matrix4 a, Matrix4 b)`
* `Matrix4 multiplyScalar(float s)`
* `float determinant()`
* `Matrix4 transpose()`
* `Matrix4 setPosition(Vector3 v)`
* `Matrix4 invert()`
* `Matrix4 scale(Vector3 v)`
* `float getMaxScaleOnAxis()`
* `Matrix4 makeTranslation(Vector3 v)`
* `Matrix4 makeRotationX(float theta)`
* `Matrix4 makeRotationY(float theta)`
* `Matrix4 makeRotationZ(float theta)`
* `Matrix4 makeRotationAxis(Vector3 axis, float angle)`
* `Matrix4 makeScale(float x, float y, float z)`
* `Matrix4 makeShear(float xy, float xz, float yx, float yz, float zx, float zy)`
* `Matrix4 makePerspective(float left, float right, float top, float bottom, float near, float far, int coordinateSystem, bool reversedDepth)`
* `Matrix4 makeOrthographic(float left, float right, float top, float bottom, float near, float far, int coordinateSystem, bool reversedDepth)`
* `bool equals(Matrix4 matrix)`
* `Matrix4 fromArray(any array, float offset)`
* `[]float toArray()`
* `any dts_make(any self, string name, ..any args)`
* `void dts_skip()`
* `void dts_adopt(any o, string name)`
* `any dts_cached(any o, string name)`
* `bool dts_nil(any v)`
* `any dts_u(any v)`
* `any dts_null()`
* `void init(BufferGeometry geometry, any material)`
* `bool isMesh()`
* `BufferGeometry geometry()`
* `void set_geometry(BufferGeometry v)`
* `any material()`
* `void set_material(any v)`
* `bool has_morphTargetInfluences()`
* `[]float morphTargetInfluences()`
* `void set_morphTargetInfluences([]float v)`
* `void clear_morphTargetInfluences()`
* `bool has_morphTargetDictionary()`
* `any morphTargetDictionary()`
* `void set_morphTargetDictionary(any v)`
* `void clear_morphTargetDictionary()`
* `float count()`
* `void set_count(float v)`
* `void updateMorphTargets()`
* `Vector3 getVertexPosition(float index, Vector3 target)`
* `any dts_make(any self, string name, ..any args)`
* `void dts_skip()`
* `void dts_adopt(any o, string name)`
* `any dts_cached(any o, string name)`
* `bool dts_nil(any v)`
* `any dts_obj({string: any} m)`
* `void init({string: any} parameters)`
* `Color color()`
* `void set_color(Color v)`
* `float lightMapIntensity()`
* `void set_lightMapIntensity(float v)`
* `float aoMapIntensity()`
* `void set_aoMapIntensity(float v)`
* `Euler envMapRotation()`
* `void set_envMapRotation(Euler v)`
* `int combine()`
* `void set_combine(int v)`
* `float reflectivity()`
* `void set_reflectivity(float v)`
* `float refractionRatio()`
* `void set_refractionRatio(float v)`
* `bool wireframe()`
* `void set_wireframe(bool v)`
* `float wireframeLinewidth()`
* `void set_wireframeLinewidth(float v)`
* `string wireframeLinecap()`
* `void set_wireframeLinecap(string v)`
* `string wireframeLinejoin()`
* `void set_wireframeLinejoin(string v)`
* `bool fog()`
* `void set_fog(bool v)`
* `bool isMeshBasicMaterial()`
* `any dts_make(any self, string name, ..any args)`
* `void dts_skip()`
* `void dts_adopt(any o, string name)`
* `any dts_cached(any o, string name)`
* `bool dts_nil(any v)`
* `any dts_obj({string: any} m)`
* `void init({string: any} parameters)`
* `Color color()`
* `void set_color(Color v)`
* `float roughness()`
* `void set_roughness(float v)`
* `float metalness()`
* `void set_metalness(float v)`
* `float lightMapIntensity()`
* `void set_lightMapIntensity(float v)`
* `float aoMapIntensity()`
* `void set_aoMapIntensity(float v)`
* `Color emissive()`
* `void set_emissive(Color v)`
* `float emissiveIntensity()`
* `void set_emissiveIntensity(float v)`
* `float bumpScale()`
* `void set_bumpScale(float v)`
* `int normalMapType()`
* `void set_normalMapType(int v)`
* `float displacementScale()`
* `void set_displacementScale(float v)`
* `float displacementBias()`
* `void set_displacementBias(float v)`
* `Euler envMapRotation()`
* `void set_envMapRotation(Euler v)`
* `float envMapIntensity()`
* `void set_envMapIntensity(float v)`
* `bool wireframe()`
* `void set_wireframe(bool v)`
* `float wireframeLinewidth()`
* `void set_wireframeLinewidth(float v)`
* `string wireframeLinecap()`
* `void set_wireframeLinecap(string v)`
* `string wireframeLinejoin()`
* `void set_wireframeLinejoin(string v)`
* `bool flatShading()`
* `void set_flatShading(bool v)`
* `bool fog()`
* `void set_fog(bool v)`
* `bool isMeshStandardMaterial()`
* `any dts_make(any self, string name, ..any args)`
* `void dts_adopt(any o, string name)`
* `any dts_cached(any o, string name)`
* `bool dts_nil(any v)`
* `any dts_u(any v)`
* `any dts_hs(any xs)`
* `any dts_cb(any target, string method)`
* `any dts_null()`
* `any h = 0`
* `void init()`
* `bool isObject3D()`
* `int id()`
* `string uuid()`
* `void set_uuid(string v)`
* `string name()`
* `void set_name(string v)`
* `string type_()`
* `bool has_parent()`
* `Object3D parent()`
* `void set_parent(Object3D v)`
* `void clear_parent()`
* `[]Object3D children()`
* `void set_children([]Object3D v)`
* `Vector3 up()`
* `void set_up(Vector3 v)`
* `Vector3 position()`
* `Euler rotation()`
* `Vector3 scale()`
* `Matrix4 modelViewMatrix()`
* `Matrix4 matrix()`
* `void set_matrix(Matrix4 v)`
* `Matrix4 matrixWorld()`
* `void set_matrixWorld(Matrix4 v)`
* `bool matrixAutoUpdate()`
* `void set_matrixAutoUpdate(bool v)`
* `bool matrixWorldAutoUpdate()`
* `void set_matrixWorldAutoUpdate(bool v)`
* `bool matrixWorldNeedsUpdate()`
* `void set_matrixWorldNeedsUpdate(bool v)`
* `bool visible()`
* `void set_visible(bool v)`
* `bool castShadow()`
* `void set_castShadow(bool v)`
* `bool receiveShadow()`
* `void set_receiveShadow(bool v)`
* `bool frustumCulled()`
* `void set_frustumCulled(bool v)`
* `float renderOrder()`
* `void set_renderOrder(float v)`
* `bool has_customDepthMaterial()`
* `Material customDepthMaterial()`
* `void set_customDepthMaterial(Material v)`
* `void clear_customDepthMaterial()`
* `bool has_customDistanceMaterial()`
* `Material customDistanceMaterial()`
* `void set_customDistanceMaterial(Material v)`
* `void clear_customDistanceMaterial()`
* `bool static()`
* `void set_static(bool v)`
* `any userData()`
* `void set_userData(any v)`
* `bool has_pivot()`
* `Vector3 pivot()`
* `void set_pivot(Vector3 v)`
* `void clear_pivot()`
* `void applyMatrix4(Matrix4 matrix)`
* `void setRotationFromAxisAngle(Vector3 axis, float angle)`
* `void setRotationFromEuler(Euler euler)`
* `void setRotationFromMatrix(Matrix4 m)`
* `Object3D rotateOnAxis(Vector3 axis, float angle)`
* `Object3D rotateOnWorldAxis(Vector3 axis, float angle)`
* `Object3D rotateX(float angle)`
* `Object3D rotateY(float angle)`
* `Object3D rotateZ(float angle)`
* `Object3D translateOnAxis(Vector3 axis, float distance)`
* `Object3D translateX(float distance)`
* `Object3D translateY(float distance)`
* `Object3D translateZ(float distance)`
* `Vector3 localToWorld(Vector3 vector)`
* `Vector3 worldToLocal(Vector3 vector)`
* `void lookAt(Vector3 vector)`
* `void lookAtXYZ(float x, float y, float z)`
* `Object3D add(Object3D object)`
* `Object3D remove(Object3D object)`
* `Object3D removeFromParent()`
* `Object3D clear()`
* `Object3D attach(Object3D object)`
* `[]Object3D getObjectsByProperty(string name, any value, []Object3D optionalTarget)`
* `Vector3 getWorldPosition(Vector3 target)`
* `Vector3 getWorldScale(Vector3 target)`
* `Vector3 getWorldDirection(Vector3 target)`
* `void traverse(any callback_target, string callback_method)`
* `void traverseVisible(any callback_target, string callback_method)`
* `void traverseAncestors(any callback_target, string callback_method)`
* `void updateMatrix()`
* `void updateMatrixWorld(bool force)`
* `void updateWorldMatrix(bool updateParents, bool updateChildren)`
* `any toJSON(any meta)`
* `Object3D clone(bool recursive = true)`
* `Object3D copy(Object3D object, bool recursive = true)`
* `any dts_make(any self, string name, ..any args)`
* `void dts_skip()`
* `bool dts_nil(any v)`
* `any dts_u(any v)`
* `any dts_null()`
* `void init(float fov = 50.0, float aspect = 1.0, float near = 0.1, float far = 2000.0)`
* `bool isPerspectiveCamera()`
* `float fov()`
* `void set_fov(float v)`
* `float zoom()`
* `void set_zoom(float v)`
* `float near()`
* `void set_near(float v)`
* `float far()`
* `void set_far(float v)`
* `float focus()`
* `void set_focus(float v)`
* `float aspect()`
* `void set_aspect(float v)`
* `bool has_view()`
* `any view_()`
* `void set_view(any v)`
* `void clear_view()`
* `float filmGauge()`
* `void set_filmGauge(float v)`
* `float filmOffset()`
* `void set_filmOffset(float v)`
* `void setFocalLength(float focalLength)`
* `float getFocalLength()`
* `float getEffectiveFOV()`
* `float getFilmWidth()`
* `float getFilmHeight()`
* `void setViewOffset(float fullWidth, float fullHeight, float x, float y, float width, float height)`
* `void clearViewOffset()`
* `void updateProjectionMatrix()`
* `any dts_make(any self, string name, ..any args)`
* `void dts_skip()`
* `void init(float width = 1.0, float height = 1.0, int widthSegments = 1, int heightSegments = 1)`
* `any parameters()`
* `any dts_make(any self, string name, ..any args)`
* `void dts_skip()`
* `bool dts_nil(any v)`
* `any dts_u(any v)`
* `any dts_null()`
* `void init()`
* `bool isScene()`
* `bool has_background()`
* `any background()`
* `void set_background(any v)`
* `void clear_background()`
* `bool has_fog()`
* `any fog()`
* `void set_fog(any v)`
* `void clear_fog()`
* `float backgroundBlurriness()`
* `void set_backgroundBlurriness(float v)`
* `float backgroundIntensity()`
* `void set_backgroundIntensity(float v)`
* `Euler backgroundRotation()`
* `void set_backgroundRotation(Euler v)`
* `float environmentIntensity()`
* `void set_environmentIntensity(float v)`
* `Euler environmentRotation()`
* `void set_environmentRotation(Euler v)`
* `bool has_overrideMaterial()`
* `Material overrideMaterial()`
* `void set_overrideMaterial(Material v)`
* `void clear_overrideMaterial()`
* `any dts_make(any self, string name, ..any args)`
* `void dts_skip()`
* `bool dts_nil(any v)`
* `any dts_u(any v)`
* `any dts_obj({string: any} m)`
* `any dts_null()`
* `void init({string: any} parameters)`
* `any uniforms()`
* `void set_uniforms(any v)`
* `string vertexShader()`
* `void set_vertexShader(string v)`
* `string fragmentShader()`
* `void set_fragmentShader(string v)`
* `float linewidth()`
* `void set_linewidth(float v)`
* `bool wireframe()`
* `void set_wireframe(bool v)`
* `float wireframeLinewidth()`
* `void set_wireframeLinewidth(float v)`
* `bool fog()`
* `void set_fog(bool v)`
* `bool lights()`
* `void set_lights(bool v)`
* `bool clipping()`
* `void set_clipping(bool v)`
* `any extensions()`
* `void set_extensions(any v)`
* `any defaultAttributeValues()`
* `void set_defaultAttributeValues(any v)`
* `bool has_index0AttributeName()`
* `string index0AttributeName()`
* `void set_index0AttributeName(string v)`
* `void clear_index0AttributeName()`
* `bool uniformsNeedUpdate()`
* `void set_uniformsNeedUpdate(bool v)`
* `bool has_glslVersion()`
* `string glslVersion()`
* `void set_glslVersion(string v)`
* `void clear_glslVersion()`
* `bool isShaderMaterial()`
* `any dts_make(any self, string name, ..any args)`
* `void dts_adopt(any o, string name)`
* `any dts_cached(any o, string name)`
* `bool dts_nil(any v)`
* `any dts_u(any v)`
* `any h = 0`
* `void init(float x, float y, float z)`
* `float x()`
* `void set_x(float v)`
* `float y()`
* `void set_y(float v)`
* `float z()`
* `void set_z(float v)`
* `bool isVector3()`
* `Vector3 set(float x, float y, float z)`
* `Vector3 setScalar(float scalar)`
* `Vector3 setX(float x)`
* `Vector3 setY(float y)`
* `Vector3 setZ(float z)`
* `Vector3 setComponent(float index, float value)`
* `float getComponent(float index)`
* `Vector3 clone()`
* `Vector3 copy(Vector3 v)`
* `Vector3 add(Vector3 v)`
* `Vector3 addScalar(float s)`
* `Vector3 addVectors(Vector3 a, Vector3 b)`
* `Vector3 addScaledVector(Vector3 v, float s)`
* `Vector3 sub(Vector3 v)`
* `Vector3 subScalar(float s)`
* `Vector3 subVectors(Vector3 a, Vector3 b)`
* `Vector3 multiply(Vector3 v)`
* `Vector3 multiplyScalar(float s)`
* `Vector3 multiplyVectors(Vector3 a, Vector3 b)`
* `Vector3 applyEuler(Euler euler)`
* `Vector3 applyAxisAngle(Vector3 axis, float angle)`
* `Vector3 applyMatrix4(Matrix4 m)`
* `Vector3 applyQuaternion(any q)`
* `Vector3 project(Camera camera)`
* `Vector3 unproject(Camera camera)`
* `Vector3 transformDirection(Matrix4 m)`
* `Vector3 divide(Vector3 v)`
* `Vector3 divideScalar(float s)`
* `Vector3 min(Vector3 v)`
* `Vector3 max(Vector3 v)`
* `Vector3 clamp(Vector3 min, Vector3 max)`
* `Vector3 clampScalar(float min, float max)`
* `Vector3 clampLength(float min, float max)`
* `Vector3 floor()`
* `Vector3 ceil()`
* `Vector3 round()`
* `Vector3 roundToZero()`
* `Vector3 negate()`
* `float dot(Vector3 v)`
* `float lengthSq()`
* `float length()`
* `float manhattanLength()`
* `Vector3 normalize()`
* `Vector3 setLength(float l)`
* `Vector3 lerp(Vector3 v, float alpha)`
* `Vector3 lerpVectors(Vector3 v1, Vector3 v2, float alpha)`
* `Vector3 cross(Vector3 v)`
* `Vector3 crossVectors(Vector3 a, Vector3 b)`
* `Vector3 projectOnVector(Vector3 v)`
* `Vector3 projectOnPlane(Vector3 planeNormal)`
* `Vector3 reflect(Vector3 vector)`
* `float angleTo(Vector3 v)`
* `float distanceTo(Vector3 v)`
* `float distanceToSquared(Vector3 v)`
* `float manhattanDistanceTo(Vector3 v)`
* `Vector3 setFromSphericalCoords(float r, float phi, float theta)`
* `Vector3 setFromCylindricalCoords(float radius, float theta, float y)`
* `Vector3 setFromMatrixPosition(Matrix4 m)`
* `Vector3 setFromMatrixScale(Matrix4 m)`
* `Vector3 setFromMatrixColumn(Matrix4 matrix, float index)`
* `Vector3 setFromEuler(Euler e)`
* `Vector3 setFromColor(any color)`
* `bool equals(Vector3 v)`
* `Vector3 fromArray(any array, float offset)`
* `[]float toArray([]float array, float offset)`
* `Vector3 fromBufferAttribute(any attribute, float index)`
* `Vector3 random()`
* `Vector3 randomDirection()`
* `any dts_make(any self, string name, ..any args)`
* `void dts_adopt(any o, string name)`
* `any dts_cached(any o, string name)`
* `bool dts_nil(any v)`
* `any dts_u(any v)`
* `any dts_obj({string: any} m)`
* `any dts_cb(any target, string method)`
* `any h = 0`
* `void init({string: any} parameters)`
* `any domElement()`
* `void set_domElement(any v)`
* `bool autoClear()`
* `void set_autoClear(bool v)`
* `bool autoClearColor()`
* `void set_autoClearColor(bool v)`
* `bool autoClearDepth()`
* `void set_autoClearDepth(bool v)`
* `bool autoClearStencil()`
* `void set_autoClearStencil(bool v)`
* `any debug()`
* `void set_debug(any v)`
* `bool sortObjects()`
* `void set_sortObjects(bool v)`
* `bool localClippingEnabled()`
* `void set_localClippingEnabled(bool v)`
* `string outputColorSpace()`
* `void set_outputColorSpace(string v)`
* `int coordinateSystem()`
* `int toneMapping()`
* `void set_toneMapping(int v)`
* `float toneMappingExposure()`
* `void set_toneMappingExposure(float v)`
* `float transmissionResolutionScale()`
* `void set_transmissionResolutionScale(float v)`
* `any shadowMap()`
* `void set_shadowMap(any v)`
* `any getContext()`
* `any getContextAttributes()`
* `void forceContextLoss()`
* `void forceContextRestore()`
* `float getPixelRatio()`
* `void setPixelRatio(float value)`
* `void setSize(float width, float height, bool updateStyle = true)`
* `void setDrawingBufferSize(float width, float height, float pixelRatio)`
* `void setEffects([]any effects)`
* `void setViewport(any x, float y, float width, float height)`
* `void setScissor(any x, float y, float width, float height)`
* `bool getScissorTest()`
* `void setScissorTest(bool enable)`
* `void setOpaqueSort(any method_target, string method_method)`
* `void setTransparentSort(any method_target, string method_method)`
* `Color getClearColor(Color target)`
* `void setClearColor(any color, float alpha = 1.0)`
* `float getClearAlpha()`
* `void setClearAlpha(float alpha)`
* `void clear(bool color, bool depth, bool stencil)`
* `void clearColor()`
* `void clearDepth()`
* `void clearStencil()`
* `void setNodesHandler(any nodesHandler)`
* `void dispose()`
* `void renderBufferDirect(Camera camera, Scene scene, BufferGeometry geometry, Material material, Object3D object, any group)`
* `void setAnimationLoop(any callback_target, string callback_method)`
* `void render(Object3D scene, Camera camera)`
* `float getActiveCubeFace()`
* `float getActiveMipmapLevel()`
* `void resetState()`
* `i64 PCFShadowMap()` — dtsgen:begin consts — regenerated; edits between the markers are lost…
* `i64 PCFSoftShadowMap()`
* `string8 SRGBColorSpace()`
* `i64 FrontSide()`
* `i64 BackSide()`
* `i64 DoubleSide()`
