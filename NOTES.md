# Technical Notes

## Debugging

* Use macro `LGL_PrintGLError` to print any warning string to standard error output if the latest OpenGL operation has any error. It's optimized in case of `NDEBUG`.
* Use `LGL_NODEBUG` if you don't want error to be printed ont on console.
* Use `LGL_AnyGLErrorMsgOnly` if you don't need returned status code of latest OpenGL operation. It's optimized when `LGL_NODEBUG` is defined, so no real function call happened there.

## Shader

* `Shader::GetUniformLocation()` will cache new shader's uniform variable names for future optimized in getting location.
* Vertex attributes' locations are set via `layout (location = ...)` inside GLSL code, but for uniform variables which will be used naming to infer and no need to do anything other than calling `Shader::GetUniformLocation()`.
