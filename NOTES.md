# Technical Notes

* Use macro `LGL_PrintGLError` to print any warning string to standard error output if the latest OpenGL operation has any error. It's optimized in case of `NDEBUG`.
* Use `LGL_NODEBUG` if you don't want error to be printed ont on console.
* Use `LGL_AnyGLErrorMsgOnly` if you don't need returned status code of latest OpenGL operation. It's optimized when `LGL_NODEBUG` is defined, so no real function call happened there.
* `Shader::GetUniformLocation()` will cache new shader's uniform variable names for future optimized in getting location.
