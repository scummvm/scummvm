*lib:
-lc %{!static:-ldll}

*libgcc:
-lsupc++ -lgcc

*link:
%{mdll:%{shared: %eshared and mdll are not compatible} %{static: %estatic and mdll are not compatible}} -L%:getenv(KOS32_SDK_DIR /lib) %{shared|mdll: -shared --entry _DllStartup} %{shared:-T%:getenv(KOS32_SDK_DIR /sources/newlib/dll.lds)} %{static: -static -T%:getenv(KOS32_SDK_DIR /sources/newlib/app.lds)} %{!mdll:%{!static:%{!shared: -call_shared -T%:getenv(KOS32_SDK_DIR /sources/newlib/app-dynamic.lds)}}} %{!mdll:-s --image-base 0} %{mdll:--enable-auto-image-base} %(shared_libgcc_undefs)

*startfile:


*endfile:


