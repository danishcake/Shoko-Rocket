#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include "vmath.h"

class Logger
{
private:
	Logger(std::string _filename);
	std::ofstream output_;

public:
	static Logger& ErrorOut();
	static Logger& DiagnosticOut();
	~Logger(void);

	Logger& Logger::operator <<( int i );
	Logger& Logger::operator <<( unsigned int i );
	Logger& Logger::operator <<( float i );
	Logger& Logger::operator <<( double i );
	Logger& Logger::operator <<( std::string i );
	Logger& Logger::operator <<(Vector3f v);
	Logger& Logger::operator <<(Vector2f v);
};
