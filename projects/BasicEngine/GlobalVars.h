#pragma once
#include "Graphics/Shader.h"


class GlobalVars
{
public:
	static Shader::Sptr s_basicShader;

	Shader::Sptr getBasicShader();
	void setBasicShader(Shader::Sptr shader);

};