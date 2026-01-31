module;


export module Dumpling;
export import DumplingForm;
export import DumplingMath;
export import DumplingRenderer;

#ifdef DUMPLING_WITH_IMGUI
export import DumplingImGui;
#endif // DUMPLING_WITH_IMGUI

#ifdef DUMPLING_WITH_HLSL_COMPLIER
export import DumplingHLSLComplier;
#endif