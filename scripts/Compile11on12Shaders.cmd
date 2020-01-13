del tmp.txt
del ..\inc\11on12shaders.h

fxc /Tvs_5_0 /EVSMain /Vn g_GenMipsVS GenerateMips.hlsl /Fh tmp.txt
type tmp.txt >> ..\inc\11on12shaders.h

fxc /Tps_5_0 /EPSMain /Vn g_GenMipsPS1D /DTex1D=1 GenerateMips.hlsl /Fh tmp.txt
type tmp.txt >> ..\inc\11on12shaders.h

fxc /Tps_5_0 /EPSMain /Vn g_GenMipsPS2D /DTex2D=1 GenerateMips.hlsl /Fh tmp.txt
type tmp.txt >> ..\inc\11on12shaders.h

fxc /Tps_5_0 /EPSMain /Vn g_GenMipsPS3D /DTex3D=1 GenerateMips.hlsl /Fh tmp.txt
type tmp.txt >> ..\inc\11on12shaders.h

fxc /Tcs_5_0 /EDrawAutoCS /Vn g_DrawAutoCS DrawAuto.hlsl /Fh tmp.txt
type tmp.txt >> ..\inc\11on12shaders.h

fxc /Tcs_5_0 /EFormatQueryCS /Vn g_FormatQueryCS FormatQuery.hlsl /Fh tmp.txt
type tmp.txt >> ..\inc\11on12shaders.h

fxc /Tcs_5_0 /EAccumulateQueryCS /Vn g_AccumulateQueryCS FormatQuery.hlsl /Fh tmp.txt
type tmp.txt >> ..\inc\11on12shaders.h

del tmp.txt