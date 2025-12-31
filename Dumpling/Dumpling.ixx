module;


export module Dumpling;
export import DumplingPlatform;
export import DumplingFormEvent;
export import DumplingForm;
export import DumplingRendererTypes;
export import DumplingRenderer;
export import DumplingPipeline;
export import DumplingShader;
export import DumplingMaterial;

#ifdef DUMPLING_WITH_IMGUI
export import DumplingImGui;
#endif // DUMPLING_WITH_IMGUI

#ifdef DUMPLING_WITH_HLSL_COMPLIER
export import DumplingHLSLComplier;
#endif