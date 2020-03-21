#pragma once

#include "../Gui/../../Potato/parser.h"
namespace Dumpling
{

	struct material_shader_code
	{
		struct material
		{
			/*
			std::wstring Define;
			std::wstring Output;
			std::vector<std::tuple<std::wstring, std::wstring>> OutputSolt;
			std::map<std::wstring, std::variant<float, bool, int, std::wstring>> Setting;
			std::map<std::wstring, std::wstring> Define;
			*/
		};

		std::map<std::wstring, material> AllMaterial;
		static material_shader_code analyze(const std::wstring& code);
	};








}