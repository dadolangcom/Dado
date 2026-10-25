<!-- dadoc 1.0.0-rc.8 -->
<!-- commit 6c56ce20508a (dirty) -->
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

913 declarations, 791 public.

* `class BoxGeometry`
* `void BoxGeometry.init(float width = 1.0, float height = 1.0, float depth = 1.0, int widthSegments = 1, int heightSegments = 1, int depthSegments = 1)`
* `any BoxGeometry.parameters()`
* `class BufferAttribute`
* `any h = 0`
* `void BufferAttribute.init(any array, float itemSize, bool normalized = false)`
* `float BufferAttribute.id()`
* `string BufferAttribute.name()`
* `void BufferAttribute.set_name(string v)`
* `any BufferAttribute.array()`
* `void BufferAttribute.set_array(any v)`
* `float BufferAttribute.itemSize()`
* `void BufferAttribute.set_itemSize(float v)`
* `int BufferAttribute.usage()`
* `void BufferAttribute.set_usage(int v)`
* `int BufferAttribute.gpuType()`
* `void BufferAttribute.set_gpuType(int v)`
* `[]any BufferAttribute.updateRanges()`
* `void BufferAttribute.set_updateRanges([]any v)`
* `float BufferAttribute.version()`
* `void BufferAttribute.set_version(float v)`
* `bool BufferAttribute.normalized()`
* `void BufferAttribute.set_normalized(bool v)`
* `float BufferAttribute.count()`
* `bool BufferAttribute.needsUpdate()`
* `void BufferAttribute.set_needsUpdate(bool v)`
* `bool BufferAttribute.isBufferAttribute()`
* `BufferAttribute BufferAttribute.onUpload(any callback_target, string callback_method)`
* `BufferAttribute BufferAttribute.setUsage(int usage)`
* `void BufferAttribute.addUpdateRange(float start, float count)`
* `void BufferAttribute.clearUpdateRanges()`
* `BufferAttribute BufferAttribute.clone()`
* `BufferAttribute BufferAttribute.copy(BufferAttribute source)`
* `BufferAttribute BufferAttribute.copyAt(float index1, BufferAttribute attribute, float index2)`
* `BufferAttribute BufferAttribute.copyArray(any array)`
* `BufferAttribute BufferAttribute.applyMatrix4(Matrix4 m)`
* `BufferAttribute BufferAttribute.transformDirection(Matrix4 m)`
* `BufferAttribute BufferAttribute.set(any value, float offset = 0.0)`
* `float BufferAttribute.getComponent(float index, float component)`
* `void BufferAttribute.setComponent(float index, float component, float value)`
* `float BufferAttribute.getX(float index)`
* `BufferAttribute BufferAttribute.setX(float index, float x)`
* `float BufferAttribute.getY(float index)`
* `BufferAttribute BufferAttribute.setY(float index, float y)`
* `float BufferAttribute.getZ(float index)`
* `BufferAttribute BufferAttribute.setZ(float index, float z)`
* `float BufferAttribute.getW(float index)`
* `BufferAttribute BufferAttribute.setW(float index, float z)`
* `BufferAttribute BufferAttribute.setXY(float index, float x, float y)`
* `BufferAttribute BufferAttribute.setXYZ(float index, float x, float y, float z)`
* `BufferAttribute BufferAttribute.setXYZW(float index, float x, float y, float z, float w)`
* `any BufferAttribute.toJSON()`
* `void BufferAttribute.dispose()`
* `class BufferGeometry`
* `void BufferGeometry.init()`
* `int BufferGeometry.id()`
* `void BufferGeometry.set_id(int v)`
* `string BufferGeometry.uuid()`
* `void BufferGeometry.set_uuid(string v)`
* `string BufferGeometry.name()`
* `void BufferGeometry.set_name(string v)`
* `string BufferGeometry.type_()`
* `bool BufferGeometry.has_index()`
* `BufferAttribute BufferGeometry.index()`
* `void BufferGeometry.set_index(BufferAttribute v)`
* `void BufferGeometry.clear_index()`
* `bool BufferGeometry.has_indirect()`
* `any BufferGeometry.indirect()`
* `void BufferGeometry.set_indirect(any v)`
* `void BufferGeometry.clear_indirect()`
* `any BufferGeometry.indirectOffset()`
* `void BufferGeometry.set_indirectOffset(any v)`
* `any BufferGeometry.attributes()`
* `void BufferGeometry.set_attributes(any v)`
* `any BufferGeometry.morphAttributes()`
* `void BufferGeometry.set_morphAttributes(any v)`
* `bool BufferGeometry.morphTargetsRelative()`
* `void BufferGeometry.set_morphTargetsRelative(bool v)`
* `[]any BufferGeometry.groups()`
* `void BufferGeometry.set_groups([]any v)`
* `any BufferGeometry.drawRange()`
* `void BufferGeometry.set_drawRange(any v)`
* `any BufferGeometry.userData()`
* `void BufferGeometry.set_userData(any v)`
* `bool BufferGeometry.isBufferGeometry()`
* `BufferGeometry BufferGeometry.setIndex(any index)`
* `BufferGeometry BufferGeometry.setIndirect(any indirect, any indirectOffset)`
* `any BufferGeometry.getIndirect()`
* `void BufferGeometry.addGroup(float start, float count, float materialIndex)`
* `void BufferGeometry.clearGroups()`
* `void BufferGeometry.setDrawRange(float start, float count)`
* `BufferGeometry BufferGeometry.applyMatrix4(Matrix4 matrix)`
* `BufferGeometry BufferGeometry.rotateX(float angle)`
* `BufferGeometry BufferGeometry.rotateY(float angle)`
* `BufferGeometry BufferGeometry.rotateZ(float angle)`
* `BufferGeometry BufferGeometry.translate(float x, float y, float z)`
* `BufferGeometry BufferGeometry.scale(float x, float y, float z)`
* `BufferGeometry BufferGeometry.lookAt(Vector3 vector)`
* `BufferGeometry BufferGeometry.center()`
* `BufferGeometry BufferGeometry.setFromPoints(any points)`
* `void BufferGeometry.computeBoundingBox()`
* `void BufferGeometry.computeBoundingSphere()`
* `void BufferGeometry.computeTangents()`
* `void BufferGeometry.computeVertexNormals()`
* `void BufferGeometry.normalizeNormals()`
* `BufferGeometry BufferGeometry.toNonIndexed()`
* `any BufferGeometry.toJSON()`
* `BufferGeometry BufferGeometry.clone()`
* `BufferGeometry BufferGeometry.copy(BufferGeometry source)`
* `void BufferGeometry.dispose()`
* `class Camera`
* `void Camera.init()`
* `bool Camera.isCamera()`
* `Matrix4 Camera.matrixWorldInverse()`
* `void Camera.set_matrixWorldInverse(Matrix4 v)`
* `Matrix4 Camera.projectionMatrix()`
* `void Camera.set_projectionMatrix(Matrix4 v)`
* `Matrix4 Camera.projectionMatrixInverse()`
* `void Camera.set_projectionMatrixInverse(Matrix4 v)`
* `int Camera.coordinateSystem()`
* `void Camera.set_coordinateSystem(int v)`
* `bool Camera.reversedDepth()`
* `class Color`
* `void Color.init(float r, float g, float b)`
* `bool Color.isColor()`
* `float Color.r()`
* `void Color.set_r(float v)`
* `float Color.g()`
* `void Color.set_g(float v)`
* `float Color.b()`
* `void Color.set_b(float v)`
* `Color Color.setFromVector3(Vector3 vector)`
* `Color Color.setScalar(float scalar)`
* `Color Color.setHex(int hex)`
* `Color Color.setRGB(float r, float g, float b, string colorSpace)`
* `Color Color.setHSL(float h_, float s, float l, string colorSpace)`
* `Color Color.setStyle(string style, string colorSpace)`
* `Color Color.setColorName(string style, string colorSpace)`
* `Color Color.clone()`
* `Color Color.copy(Color color)`
* `Color Color.copySRGBToLinear(Color color)`
* `Color Color.copyLinearToSRGB(Color color)`
* `Color Color.convertSRGBToLinear()`
* `Color Color.convertLinearToSRGB()`
* `int Color.getHex()`
* `string Color.getHexString()`
* `any Color.getHSL(any target, string colorSpace)`
* `any Color.getRGB(any target, string colorSpace)`
* `string Color.getStyle()`
* `Color Color.offsetHSL(float h_, float s, float l)`
* `Color Color.add(Color color)`
* `Color Color.addColors(Color color1, Color color2)`
* `Color Color.addScalar(float s)`
* `Color Color.sub(Color color)`
* `Color Color.multiply(Color color)`
* `Color Color.multiplyScalar(float s)`
* `Color Color.lerp(Color color, float alpha)`
* `Color Color.lerpColors(Color color1, Color color2, float alpha)`
* `Color Color.lerpHSL(Color color, float alpha)`
* `bool Color.equals(Color color)`
* `Color Color.fromArray(any array, float offset)`
* `[]float Color.toArray([]float array, float offset)`
* `float Color.toJSON()`
* `Color Color.fromBufferAttribute(any attribute, float index)`
* `class DirectionalLight`
* `void DirectionalLight.init(any color = 16777215, float intensity = 1.0)`
* `bool DirectionalLight.isDirectionalLight()`
* `Object3D DirectionalLight.target()`
* `void DirectionalLight.set_target(Object3D v)`
* `any DirectionalLight.shadow()`
* `void DirectionalLight.set_shadow(any v)`
* `class Euler`
* `void Euler.init(float x, float y, float z, string order)`
* `float Euler.x()`
* `void Euler.set_x(float v)`
* `float Euler.y()`
* `void Euler.set_y(float v)`
* `float Euler.z()`
* `void Euler.set_z(float v)`
* `string Euler.order()`
* `void Euler.set_order(string v)`
* `bool Euler.isEuler()`
* `Euler Euler.set(float x, float y, float z, string order)`
* `Euler Euler.clone()`
* `Euler Euler.copy(Euler euler)`
* `Euler Euler.setFromRotationMatrix(Matrix4 m, string order, bool update)`
* `Euler Euler.setFromVector3(Vector3 v, string order)`
* `Euler Euler.reorder(string newOrder)`
* `bool Euler.equals(Euler euler)`
* `Euler Euler.fromArray(any array)`
* `any Euler.toArray(any array, float offset)`
* `Euler Euler._onChange(any callback_target, string callback_method)`
* `class GridHelper`
* `void GridHelper.init(float size = 10.0, int divisions = 10, any color1 = 4473924, any color2 = 8947848)`
* `void GridHelper.dispose()`
* `bool GridHelper.isLineSegments()`
* `bool GridHelper.isLine()`
* `BufferGeometry GridHelper.geometry()`
* `void GridHelper.set_geometry(BufferGeometry v)`
* `bool GridHelper.has_morphTargetInfluences()`
* `[]float GridHelper.morphTargetInfluences()`
* `void GridHelper.set_morphTargetInfluences([]float v)`
* `void GridHelper.clear_morphTargetInfluences()`
* `bool GridHelper.has_morphTargetDictionary()`
* `any GridHelper.morphTargetDictionary()`
* `void GridHelper.set_morphTargetDictionary(any v)`
* `void GridHelper.clear_morphTargetDictionary()`
* `GridHelper GridHelper.computeLineDistances()`
* `void GridHelper.updateMorphTargets()`
* `class HemisphereLight`
* `void HemisphereLight.init(any skyColor = 16777215, any groundColor = 16777215, float intensity = 1.0)`
* `bool HemisphereLight.isHemisphereLight()`
* `Color HemisphereLight.groundColor()`
* `void HemisphereLight.set_groundColor(Color v)`
* `class Light`
* `void Light.init(any color = 16777215, float intensity = 1.0)`
* `bool Light.isLight()`
* `Color Light.color()`
* `void Light.set_color(Color v)`
* `float Light.intensity()`
* `void Light.set_intensity(float v)`
* `void Light.dispose()`
* `class Material`
* `void Material.init()`
* `string Material.name()`
* `void Material.set_name(string v)`
* `int Material.blending()`
* `void Material.set_blending(int v)`
* `int Material.side()`
* `void Material.set_side(int v)`
* `bool Material.vertexColors()`
* `void Material.set_vertexColors(bool v)`
* `float Material.opacity()`
* `void Material.set_opacity(float v)`
* `bool Material.transparent()`
* `void Material.set_transparent(bool v)`
* `bool Material.alphaHash()`
* `void Material.set_alphaHash(bool v)`
* `int Material.blendSrc()`
* `void Material.set_blendSrc(int v)`
* `int Material.blendDst()`
* `void Material.set_blendDst(int v)`
* `int Material.blendEquation()`
* `void Material.set_blendEquation(int v)`
* `bool Material.has_blendSrcAlpha()`
* `int Material.blendSrcAlpha()`
* `void Material.set_blendSrcAlpha(int v)`
* `void Material.clear_blendSrcAlpha()`
* `bool Material.has_blendDstAlpha()`
* `int Material.blendDstAlpha()`
* `void Material.set_blendDstAlpha(int v)`
* `void Material.clear_blendDstAlpha()`
* `bool Material.has_blendEquationAlpha()`
* `int Material.blendEquationAlpha()`
* `void Material.set_blendEquationAlpha(int v)`
* `void Material.clear_blendEquationAlpha()`
* `Color Material.blendColor()`
* `void Material.set_blendColor(Color v)`
* `float Material.blendAlpha()`
* `void Material.set_blendAlpha(float v)`
* `int Material.depthFunc()`
* `void Material.set_depthFunc(int v)`
* `bool Material.depthTest()`
* `void Material.set_depthTest(bool v)`
* `bool Material.depthWrite()`
* `void Material.set_depthWrite(bool v)`
* `float Material.stencilWriteMask()`
* `void Material.set_stencilWriteMask(float v)`
* `int Material.stencilFunc()`
* `void Material.set_stencilFunc(int v)`
* `float Material.stencilRef()`
* `void Material.set_stencilRef(float v)`
* `float Material.stencilFuncMask()`
* `void Material.set_stencilFuncMask(float v)`
* `int Material.stencilFail()`
* `void Material.set_stencilFail(int v)`
* `int Material.stencilZFail()`
* `void Material.set_stencilZFail(int v)`
* `int Material.stencilZPass()`
* `void Material.set_stencilZPass(int v)`
* `bool Material.stencilWrite()`
* `void Material.set_stencilWrite(bool v)`
* `bool Material.clipIntersection()`
* `void Material.set_clipIntersection(bool v)`
* `bool Material.clipShadows()`
* `void Material.set_clipShadows(bool v)`
* `bool Material.has_shadowSide()`
* `int Material.shadowSide()`
* `void Material.set_shadowSide(int v)`
* `void Material.clear_shadowSide()`
* `bool Material.colorWrite()`
* `void Material.set_colorWrite(bool v)`
* `bool Material.has_precision()`
* `string Material.precision()`
* `void Material.set_precision(string v)`
* `void Material.clear_precision()`
* `bool Material.polygonOffset()`
* `void Material.set_polygonOffset(bool v)`
* `float Material.polygonOffsetFactor()`
* `void Material.set_polygonOffsetFactor(float v)`
* `float Material.polygonOffsetUnits()`
* `void Material.set_polygonOffsetUnits(float v)`
* `bool Material.dithering()`
* `void Material.set_dithering(bool v)`
* `bool Material.alphaToCoverage()`
* `void Material.set_alphaToCoverage(bool v)`
* `bool Material.premultipliedAlpha()`
* `void Material.set_premultipliedAlpha(bool v)`
* `bool Material.forceSinglePass()`
* `void Material.set_forceSinglePass(bool v)`
* `bool Material.allowOverride()`
* `void Material.set_allowOverride(bool v)`
* `bool Material.visible()`
* `void Material.set_visible(bool v)`
* `bool Material.toneMapped()`
* `void Material.set_toneMapped(bool v)`
* `any Material.userData()`
* `void Material.set_userData(any v)`
* `float Material.alphaTest()`
* `void Material.set_alphaTest(float v)`
* `bool Material.isMaterial()`
* `string Material.uuid()`
* `string Material.type_()`
* `void Material.set_type(string v)`
* `float Material.version()`
* `bool Material.has_defines()`
* `any Material.defines()`
* `void Material.set_defines(any v)`
* `void Material.clear_defines()`
* `void Material.onBeforeCompile(any parameters, WebGLRenderer renderer)`
* `string Material.customProgramCacheKey()`
* `void Material.setValues({string: any} values)`
* `any Material.toJSON(any meta)`
* `Material Material.clone()`
* `Material Material.copy(Material source)`
* `void Material.dispose()`
* `bool Material.needsUpdate()`
* `void Material.set_needsUpdate(bool v)`
* `class Matrix4`
* `void Matrix4.init()`
* `bool Matrix4.isMatrix4()`
* `[]float Matrix4.elements()`
* `void Matrix4.set_elements([]float v)`
* `Matrix4 Matrix4.set(float n11, float n12, float n13, float n14, float n21, float n22, float n23, float n24, float n31, float n32, float n33, float n34, float n41, float n42, float n43, float n44)`
* `Matrix4 Matrix4.identity()`
* `Matrix4 Matrix4.clone()`
* `Matrix4 Matrix4.copy(Matrix4 m)`
* `Matrix4 Matrix4.copyPosition(Matrix4 m)`
* `Matrix4 Matrix4.extractBasis(Vector3 xAxis, Vector3 yAxis, Vector3 zAxis)`
* `Matrix4 Matrix4.makeBasis(Vector3 xAxis, Vector3 yAxis, Vector3 zAxis)`
* `Matrix4 Matrix4.extractRotation(Matrix4 m)`
* `Matrix4 Matrix4.makeRotationFromEuler(Euler euler)`
* `Matrix4 Matrix4.lookAt(Vector3 eye, Vector3 target, Vector3 up)`
* `Matrix4 Matrix4.multiply(Matrix4 m)`
* `Matrix4 Matrix4.premultiply(Matrix4 m)`
* `Matrix4 Matrix4.multiplyMatrices(Matrix4 a, Matrix4 b)`
* `Matrix4 Matrix4.multiplyScalar(float s)`
* `float Matrix4.determinant()`
* `Matrix4 Matrix4.transpose()`
* `Matrix4 Matrix4.setPosition(Vector3 v)`
* `Matrix4 Matrix4.invert()`
* `Matrix4 Matrix4.scale(Vector3 v)`
* `float Matrix4.getMaxScaleOnAxis()`
* `Matrix4 Matrix4.makeTranslation(Vector3 v)`
* `Matrix4 Matrix4.makeRotationX(float theta)`
* `Matrix4 Matrix4.makeRotationY(float theta)`
* `Matrix4 Matrix4.makeRotationZ(float theta)`
* `Matrix4 Matrix4.makeRotationAxis(Vector3 axis, float angle)`
* `Matrix4 Matrix4.makeScale(float x, float y, float z)`
* `Matrix4 Matrix4.makeShear(float xy, float xz, float yx, float yz, float zx, float zy)`
* `Matrix4 Matrix4.makePerspective(float left, float right, float top, float bottom, float near, float far, int coordinateSystem, bool reversedDepth)`
* `Matrix4 Matrix4.makeOrthographic(float left, float right, float top, float bottom, float near, float far, int coordinateSystem, bool reversedDepth)`
* `bool Matrix4.equals(Matrix4 matrix)`
* `Matrix4 Matrix4.fromArray(any array, float offset)`
* `[]float Matrix4.toArray()`
* `class Mesh`
* `void Mesh.init(BufferGeometry geometry, any material)`
* `bool Mesh.isMesh()`
* `BufferGeometry Mesh.geometry()`
* `void Mesh.set_geometry(BufferGeometry v)`
* `any Mesh.material()`
* `void Mesh.set_material(any v)`
* `bool Mesh.has_morphTargetInfluences()`
* `[]float Mesh.morphTargetInfluences()`
* `void Mesh.set_morphTargetInfluences([]float v)`
* `void Mesh.clear_morphTargetInfluences()`
* `bool Mesh.has_morphTargetDictionary()`
* `any Mesh.morphTargetDictionary()`
* `void Mesh.set_morphTargetDictionary(any v)`
* `void Mesh.clear_morphTargetDictionary()`
* `float Mesh.count()`
* `void Mesh.set_count(float v)`
* `void Mesh.updateMorphTargets()`
* `Vector3 Mesh.getVertexPosition(float index, Vector3 target)`
* `class MeshBasicMaterial`
* `void MeshBasicMaterial.init({string: any} parameters)`
* `Color MeshBasicMaterial.color()`
* `void MeshBasicMaterial.set_color(Color v)`
* `float MeshBasicMaterial.lightMapIntensity()`
* `void MeshBasicMaterial.set_lightMapIntensity(float v)`
* `float MeshBasicMaterial.aoMapIntensity()`
* `void MeshBasicMaterial.set_aoMapIntensity(float v)`
* `Euler MeshBasicMaterial.envMapRotation()`
* `void MeshBasicMaterial.set_envMapRotation(Euler v)`
* `int MeshBasicMaterial.combine()`
* `void MeshBasicMaterial.set_combine(int v)`
* `float MeshBasicMaterial.reflectivity()`
* `void MeshBasicMaterial.set_reflectivity(float v)`
* `float MeshBasicMaterial.refractionRatio()`
* `void MeshBasicMaterial.set_refractionRatio(float v)`
* `bool MeshBasicMaterial.wireframe()`
* `void MeshBasicMaterial.set_wireframe(bool v)`
* `float MeshBasicMaterial.wireframeLinewidth()`
* `void MeshBasicMaterial.set_wireframeLinewidth(float v)`
* `string MeshBasicMaterial.wireframeLinecap()`
* `void MeshBasicMaterial.set_wireframeLinecap(string v)`
* `string MeshBasicMaterial.wireframeLinejoin()`
* `void MeshBasicMaterial.set_wireframeLinejoin(string v)`
* `bool MeshBasicMaterial.fog()`
* `void MeshBasicMaterial.set_fog(bool v)`
* `bool MeshBasicMaterial.isMeshBasicMaterial()`
* `class MeshStandardMaterial`
* `void MeshStandardMaterial.init({string: any} parameters)`
* `Color MeshStandardMaterial.color()`
* `void MeshStandardMaterial.set_color(Color v)`
* `float MeshStandardMaterial.roughness()`
* `void MeshStandardMaterial.set_roughness(float v)`
* `float MeshStandardMaterial.metalness()`
* `void MeshStandardMaterial.set_metalness(float v)`
* `float MeshStandardMaterial.lightMapIntensity()`
* `void MeshStandardMaterial.set_lightMapIntensity(float v)`
* `float MeshStandardMaterial.aoMapIntensity()`
* `void MeshStandardMaterial.set_aoMapIntensity(float v)`
* `Color MeshStandardMaterial.emissive()`
* `void MeshStandardMaterial.set_emissive(Color v)`
* `float MeshStandardMaterial.emissiveIntensity()`
* `void MeshStandardMaterial.set_emissiveIntensity(float v)`
* `float MeshStandardMaterial.bumpScale()`
* `void MeshStandardMaterial.set_bumpScale(float v)`
* `int MeshStandardMaterial.normalMapType()`
* `void MeshStandardMaterial.set_normalMapType(int v)`
* `float MeshStandardMaterial.displacementScale()`
* `void MeshStandardMaterial.set_displacementScale(float v)`
* `float MeshStandardMaterial.displacementBias()`
* `void MeshStandardMaterial.set_displacementBias(float v)`
* `Euler MeshStandardMaterial.envMapRotation()`
* `void MeshStandardMaterial.set_envMapRotation(Euler v)`
* `float MeshStandardMaterial.envMapIntensity()`
* `void MeshStandardMaterial.set_envMapIntensity(float v)`
* `bool MeshStandardMaterial.wireframe()`
* `void MeshStandardMaterial.set_wireframe(bool v)`
* `float MeshStandardMaterial.wireframeLinewidth()`
* `void MeshStandardMaterial.set_wireframeLinewidth(float v)`
* `string MeshStandardMaterial.wireframeLinecap()`
* `void MeshStandardMaterial.set_wireframeLinecap(string v)`
* `string MeshStandardMaterial.wireframeLinejoin()`
* `void MeshStandardMaterial.set_wireframeLinejoin(string v)`
* `bool MeshStandardMaterial.flatShading()`
* `void MeshStandardMaterial.set_flatShading(bool v)`
* `bool MeshStandardMaterial.fog()`
* `void MeshStandardMaterial.set_fog(bool v)`
* `bool MeshStandardMaterial.isMeshStandardMaterial()`
* `class Object3D`
* `void Object3D.init()`
* `bool Object3D.isObject3D()`
* `int Object3D.id()`
* `string Object3D.uuid()`
* `void Object3D.set_uuid(string v)`
* `string Object3D.name()`
* `void Object3D.set_name(string v)`
* `string Object3D.type_()`
* `bool Object3D.has_parent()`
* `Object3D Object3D.parent()`
* `void Object3D.set_parent(Object3D v)`
* `void Object3D.clear_parent()`
* `[]Object3D Object3D.children()`
* `void Object3D.set_children([]Object3D v)`
* `Vector3 Object3D.up()`
* `void Object3D.set_up(Vector3 v)`
* `Vector3 Object3D.position()`
* `Euler Object3D.rotation()`
* `Vector3 Object3D.scale()`
* `Matrix4 Object3D.modelViewMatrix()`
* `Matrix4 Object3D.matrix()`
* `void Object3D.set_matrix(Matrix4 v)`
* `Matrix4 Object3D.matrixWorld()`
* `void Object3D.set_matrixWorld(Matrix4 v)`
* `bool Object3D.matrixAutoUpdate()`
* `void Object3D.set_matrixAutoUpdate(bool v)`
* `bool Object3D.matrixWorldAutoUpdate()`
* `void Object3D.set_matrixWorldAutoUpdate(bool v)`
* `bool Object3D.matrixWorldNeedsUpdate()`
* `void Object3D.set_matrixWorldNeedsUpdate(bool v)`
* `bool Object3D.visible()`
* `void Object3D.set_visible(bool v)`
* `bool Object3D.castShadow()`
* `void Object3D.set_castShadow(bool v)`
* `bool Object3D.receiveShadow()`
* `void Object3D.set_receiveShadow(bool v)`
* `bool Object3D.frustumCulled()`
* `void Object3D.set_frustumCulled(bool v)`
* `float Object3D.renderOrder()`
* `void Object3D.set_renderOrder(float v)`
* `bool Object3D.has_customDepthMaterial()`
* `Material Object3D.customDepthMaterial()`
* `void Object3D.set_customDepthMaterial(Material v)`
* `void Object3D.clear_customDepthMaterial()`
* `bool Object3D.has_customDistanceMaterial()`
* `Material Object3D.customDistanceMaterial()`
* `void Object3D.set_customDistanceMaterial(Material v)`
* `void Object3D.clear_customDistanceMaterial()`
* `bool Object3D.static()`
* `void Object3D.set_static(bool v)`
* `any Object3D.userData()`
* `void Object3D.set_userData(any v)`
* `bool Object3D.has_pivot()`
* `Vector3 Object3D.pivot()`
* `void Object3D.set_pivot(Vector3 v)`
* `void Object3D.clear_pivot()`
* `void Object3D.applyMatrix4(Matrix4 matrix)`
* `void Object3D.setRotationFromAxisAngle(Vector3 axis, float angle)`
* `void Object3D.setRotationFromEuler(Euler euler)`
* `void Object3D.setRotationFromMatrix(Matrix4 m)`
* `Object3D Object3D.rotateOnAxis(Vector3 axis, float angle)`
* `Object3D Object3D.rotateOnWorldAxis(Vector3 axis, float angle)`
* `Object3D Object3D.rotateX(float angle)`
* `Object3D Object3D.rotateY(float angle)`
* `Object3D Object3D.rotateZ(float angle)`
* `Object3D Object3D.translateOnAxis(Vector3 axis, float distance)`
* `Object3D Object3D.translateX(float distance)`
* `Object3D Object3D.translateY(float distance)`
* `Object3D Object3D.translateZ(float distance)`
* `Vector3 Object3D.localToWorld(Vector3 vector)`
* `Vector3 Object3D.worldToLocal(Vector3 vector)`
* `void Object3D.lookAt(Vector3 vector)`
* `void Object3D.lookAtXYZ(float x, float y, float z)`
* `Object3D Object3D.add(Object3D object)`
* `Object3D Object3D.remove(Object3D object)`
* `Object3D Object3D.removeFromParent()`
* `Object3D Object3D.clear()`
* `Object3D Object3D.attach(Object3D object)`
* `[]Object3D Object3D.getObjectsByProperty(string name, any value, []Object3D optionalTarget)`
* `Vector3 Object3D.getWorldPosition(Vector3 target)`
* `Vector3 Object3D.getWorldScale(Vector3 target)`
* `Vector3 Object3D.getWorldDirection(Vector3 target)`
* `void Object3D.traverse(any callback_target, string callback_method)`
* `void Object3D.traverseVisible(any callback_target, string callback_method)`
* `void Object3D.traverseAncestors(any callback_target, string callback_method)`
* `void Object3D.updateMatrix()`
* `void Object3D.updateMatrixWorld(bool force)`
* `void Object3D.updateWorldMatrix(bool updateParents, bool updateChildren)`
* `any Object3D.toJSON(any meta)`
* `Object3D Object3D.clone(bool recursive = true)`
* `Object3D Object3D.copy(Object3D object, bool recursive = true)`
* `class PerspectiveCamera`
* `void PerspectiveCamera.init(float fov = 50.0, float aspect = 1.0, float near = 0.1, float far = 2000.0)`
* `bool PerspectiveCamera.isPerspectiveCamera()`
* `float PerspectiveCamera.fov()`
* `void PerspectiveCamera.set_fov(float v)`
* `float PerspectiveCamera.zoom()`
* `void PerspectiveCamera.set_zoom(float v)`
* `float PerspectiveCamera.near()`
* `void PerspectiveCamera.set_near(float v)`
* `float PerspectiveCamera.far()`
* `void PerspectiveCamera.set_far(float v)`
* `float PerspectiveCamera.focus()`
* `void PerspectiveCamera.set_focus(float v)`
* `float PerspectiveCamera.aspect()`
* `void PerspectiveCamera.set_aspect(float v)`
* `bool PerspectiveCamera.has_view()`
* `any PerspectiveCamera.view_()`
* `void PerspectiveCamera.set_view(any v)`
* `void PerspectiveCamera.clear_view()`
* `float PerspectiveCamera.filmGauge()`
* `void PerspectiveCamera.set_filmGauge(float v)`
* `float PerspectiveCamera.filmOffset()`
* `void PerspectiveCamera.set_filmOffset(float v)`
* `void PerspectiveCamera.setFocalLength(float focalLength)`
* `float PerspectiveCamera.getFocalLength()`
* `float PerspectiveCamera.getEffectiveFOV()`
* `float PerspectiveCamera.getFilmWidth()`
* `float PerspectiveCamera.getFilmHeight()`
* `void PerspectiveCamera.setViewOffset(float fullWidth, float fullHeight, float x, float y, float width, float height)`
* `void PerspectiveCamera.clearViewOffset()`
* `void PerspectiveCamera.updateProjectionMatrix()`
* `class PlaneGeometry`
* `void PlaneGeometry.init(float width = 1.0, float height = 1.0, int widthSegments = 1, int heightSegments = 1)`
* `any PlaneGeometry.parameters()`
* `class Scene`
* `void Scene.init()`
* `bool Scene.isScene()`
* `bool Scene.has_background()`
* `any Scene.background()`
* `void Scene.set_background(any v)`
* `void Scene.clear_background()`
* `bool Scene.has_fog()`
* `any Scene.fog()`
* `void Scene.set_fog(any v)`
* `void Scene.clear_fog()`
* `float Scene.backgroundBlurriness()`
* `void Scene.set_backgroundBlurriness(float v)`
* `float Scene.backgroundIntensity()`
* `void Scene.set_backgroundIntensity(float v)`
* `Euler Scene.backgroundRotation()`
* `void Scene.set_backgroundRotation(Euler v)`
* `float Scene.environmentIntensity()`
* `void Scene.set_environmentIntensity(float v)`
* `Euler Scene.environmentRotation()`
* `void Scene.set_environmentRotation(Euler v)`
* `bool Scene.has_overrideMaterial()`
* `Material Scene.overrideMaterial()`
* `void Scene.set_overrideMaterial(Material v)`
* `void Scene.clear_overrideMaterial()`
* `class ShaderMaterial`
* `void ShaderMaterial.init({string: any} parameters)`
* `any ShaderMaterial.uniforms()`
* `void ShaderMaterial.set_uniforms(any v)`
* `string ShaderMaterial.vertexShader()`
* `void ShaderMaterial.set_vertexShader(string v)`
* `string ShaderMaterial.fragmentShader()`
* `void ShaderMaterial.set_fragmentShader(string v)`
* `float ShaderMaterial.linewidth()`
* `void ShaderMaterial.set_linewidth(float v)`
* `bool ShaderMaterial.wireframe()`
* `void ShaderMaterial.set_wireframe(bool v)`
* `float ShaderMaterial.wireframeLinewidth()`
* `void ShaderMaterial.set_wireframeLinewidth(float v)`
* `bool ShaderMaterial.fog()`
* `void ShaderMaterial.set_fog(bool v)`
* `bool ShaderMaterial.lights()`
* `void ShaderMaterial.set_lights(bool v)`
* `bool ShaderMaterial.clipping()`
* `void ShaderMaterial.set_clipping(bool v)`
* `any ShaderMaterial.extensions()`
* `void ShaderMaterial.set_extensions(any v)`
* `any ShaderMaterial.defaultAttributeValues()`
* `void ShaderMaterial.set_defaultAttributeValues(any v)`
* `bool ShaderMaterial.has_index0AttributeName()`
* `string ShaderMaterial.index0AttributeName()`
* `void ShaderMaterial.set_index0AttributeName(string v)`
* `void ShaderMaterial.clear_index0AttributeName()`
* `bool ShaderMaterial.uniformsNeedUpdate()`
* `void ShaderMaterial.set_uniformsNeedUpdate(bool v)`
* `bool ShaderMaterial.has_glslVersion()`
* `string ShaderMaterial.glslVersion()`
* `void ShaderMaterial.set_glslVersion(string v)`
* `void ShaderMaterial.clear_glslVersion()`
* `bool ShaderMaterial.isShaderMaterial()`
* `class Vector3`
* `void Vector3.init(float x, float y, float z)`
* `float Vector3.x()`
* `void Vector3.set_x(float v)`
* `float Vector3.y()`
* `void Vector3.set_y(float v)`
* `float Vector3.z()`
* `void Vector3.set_z(float v)`
* `bool Vector3.isVector3()`
* `Vector3 Vector3.set(float x, float y, float z)`
* `Vector3 Vector3.setScalar(float scalar)`
* `Vector3 Vector3.setX(float x)`
* `Vector3 Vector3.setY(float y)`
* `Vector3 Vector3.setZ(float z)`
* `Vector3 Vector3.setComponent(float index, float value)`
* `float Vector3.getComponent(float index)`
* `Vector3 Vector3.clone()`
* `Vector3 Vector3.copy(Vector3 v)`
* `Vector3 Vector3.add(Vector3 v)`
* `Vector3 Vector3.addScalar(float s)`
* `Vector3 Vector3.addVectors(Vector3 a, Vector3 b)`
* `Vector3 Vector3.addScaledVector(Vector3 v, float s)`
* `Vector3 Vector3.sub(Vector3 v)`
* `Vector3 Vector3.subScalar(float s)`
* `Vector3 Vector3.subVectors(Vector3 a, Vector3 b)`
* `Vector3 Vector3.multiply(Vector3 v)`
* `Vector3 Vector3.multiplyScalar(float s)`
* `Vector3 Vector3.multiplyVectors(Vector3 a, Vector3 b)`
* `Vector3 Vector3.applyEuler(Euler euler)`
* `Vector3 Vector3.applyAxisAngle(Vector3 axis, float angle)`
* `Vector3 Vector3.applyMatrix4(Matrix4 m)`
* `Vector3 Vector3.applyQuaternion(any q)`
* `Vector3 Vector3.project(Camera camera)`
* `Vector3 Vector3.unproject(Camera camera)`
* `Vector3 Vector3.transformDirection(Matrix4 m)`
* `Vector3 Vector3.divide(Vector3 v)`
* `Vector3 Vector3.divideScalar(float s)`
* `Vector3 Vector3.min(Vector3 v)`
* `Vector3 Vector3.max(Vector3 v)`
* `Vector3 Vector3.clamp(Vector3 min, Vector3 max)`
* `Vector3 Vector3.clampScalar(float min, float max)`
* `Vector3 Vector3.clampLength(float min, float max)`
* `Vector3 Vector3.floor()`
* `Vector3 Vector3.ceil()`
* `Vector3 Vector3.round()`
* `Vector3 Vector3.roundToZero()`
* `Vector3 Vector3.negate()`
* `float Vector3.dot(Vector3 v)`
* `float Vector3.lengthSq()`
* `float Vector3.length()`
* `float Vector3.manhattanLength()`
* `Vector3 Vector3.normalize()`
* `Vector3 Vector3.setLength(float l)`
* `Vector3 Vector3.lerp(Vector3 v, float alpha)`
* `Vector3 Vector3.lerpVectors(Vector3 v1, Vector3 v2, float alpha)`
* `Vector3 Vector3.cross(Vector3 v)`
* `Vector3 Vector3.crossVectors(Vector3 a, Vector3 b)`
* `Vector3 Vector3.projectOnVector(Vector3 v)`
* `Vector3 Vector3.projectOnPlane(Vector3 planeNormal)`
* `Vector3 Vector3.reflect(Vector3 vector)`
* `float Vector3.angleTo(Vector3 v)`
* `float Vector3.distanceTo(Vector3 v)`
* `float Vector3.distanceToSquared(Vector3 v)`
* `float Vector3.manhattanDistanceTo(Vector3 v)`
* `Vector3 Vector3.setFromSphericalCoords(float r, float phi, float theta)`
* `Vector3 Vector3.setFromCylindricalCoords(float radius, float theta, float y)`
* `Vector3 Vector3.setFromMatrixPosition(Matrix4 m)`
* `Vector3 Vector3.setFromMatrixScale(Matrix4 m)`
* `Vector3 Vector3.setFromMatrixColumn(Matrix4 matrix, float index)`
* `Vector3 Vector3.setFromEuler(Euler e)`
* `Vector3 Vector3.setFromColor(any color)`
* `bool Vector3.equals(Vector3 v)`
* `Vector3 Vector3.fromArray(any array, float offset)`
* `[]float Vector3.toArray([]float array, float offset)`
* `Vector3 Vector3.fromBufferAttribute(any attribute, float index)`
* `Vector3 Vector3.random()`
* `Vector3 Vector3.randomDirection()`
* `class WebGLRenderer`
* `void WebGLRenderer.init({string: any} parameters)`
* `any WebGLRenderer.domElement()`
* `void WebGLRenderer.set_domElement(any v)`
* `bool WebGLRenderer.autoClear()`
* `void WebGLRenderer.set_autoClear(bool v)`
* `bool WebGLRenderer.autoClearColor()`
* `void WebGLRenderer.set_autoClearColor(bool v)`
* `bool WebGLRenderer.autoClearDepth()`
* `void WebGLRenderer.set_autoClearDepth(bool v)`
* `bool WebGLRenderer.autoClearStencil()`
* `void WebGLRenderer.set_autoClearStencil(bool v)`
* `any WebGLRenderer.debug()`
* `void WebGLRenderer.set_debug(any v)`
* `bool WebGLRenderer.sortObjects()`
* `void WebGLRenderer.set_sortObjects(bool v)`
* `bool WebGLRenderer.localClippingEnabled()`
* `void WebGLRenderer.set_localClippingEnabled(bool v)`
* `string WebGLRenderer.outputColorSpace()`
* `void WebGLRenderer.set_outputColorSpace(string v)`
* `int WebGLRenderer.coordinateSystem()`
* `int WebGLRenderer.toneMapping()`
* `void WebGLRenderer.set_toneMapping(int v)`
* `float WebGLRenderer.toneMappingExposure()`
* `void WebGLRenderer.set_toneMappingExposure(float v)`
* `float WebGLRenderer.transmissionResolutionScale()`
* `void WebGLRenderer.set_transmissionResolutionScale(float v)`
* `any WebGLRenderer.shadowMap()`
* `void WebGLRenderer.set_shadowMap(any v)`
* `any WebGLRenderer.getContext()`
* `any WebGLRenderer.getContextAttributes()`
* `void WebGLRenderer.forceContextLoss()`
* `void WebGLRenderer.forceContextRestore()`
* `float WebGLRenderer.getPixelRatio()`
* `void WebGLRenderer.setPixelRatio(float value)`
* `void WebGLRenderer.setSize(float width, float height, bool updateStyle = true)`
* `void WebGLRenderer.setDrawingBufferSize(float width, float height, float pixelRatio)`
* `void WebGLRenderer.setEffects([]any effects)`
* `void WebGLRenderer.setViewport(any x, float y, float width, float height)`
* `void WebGLRenderer.setScissor(any x, float y, float width, float height)`
* `bool WebGLRenderer.getScissorTest()`
* `void WebGLRenderer.setScissorTest(bool enable)`
* `void WebGLRenderer.setOpaqueSort(any method_target, string method_method)`
* `void WebGLRenderer.setTransparentSort(any method_target, string method_method)`
* `Color WebGLRenderer.getClearColor(Color target)`
* `void WebGLRenderer.setClearColor(any color, float alpha = 1.0)`
* `float WebGLRenderer.getClearAlpha()`
* `void WebGLRenderer.setClearAlpha(float alpha)`
* `void WebGLRenderer.clear(bool color, bool depth, bool stencil)`
* `void WebGLRenderer.clearColor()`
* `void WebGLRenderer.clearDepth()`
* `void WebGLRenderer.clearStencil()`
* `void WebGLRenderer.setNodesHandler(any nodesHandler)`
* `void WebGLRenderer.dispose()`
* `void WebGLRenderer.renderBufferDirect(Camera camera, Scene scene, BufferGeometry geometry, Material material, Object3D object, any group)`
* `void WebGLRenderer.setAnimationLoop(any callback_target, string callback_method)`
* `void WebGLRenderer.render(Object3D scene, Camera camera)`
* `float WebGLRenderer.getActiveCubeFace()`
* `float WebGLRenderer.getActiveMipmapLevel()`
* `void WebGLRenderer.resetState()`
* `i64 PCFShadowMap()` — dtsgen:begin consts — regenerated; edits between the markers are lost…
* `i64 PCFSoftShadowMap()`
* `string8 SRGBColorSpace()`
* `i64 FrontSide()`
* `i64 BackSide()`
* `i64 DoubleSide()`
