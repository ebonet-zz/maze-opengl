#ifndef __SHADERMANAGER_
#define __SHADERMANAGER_

#include <SFML/Window.hpp>
#include <stdio.h>

class ShaderManager
{
public:
	bool GL20Support;
	
	ShaderManager()
	{
		logFile = NULL;
		GL20Support= __GLEW_VERSION_2_0 ;
	}
	
	ShaderManager(FILE * f)
	{
		logFile = f;
		GL20Support= __GLEW_VERSION_2_0;
	}
	
	GLuint buildShaderProgram(char const * const * vertShaderPath, char const * const * fragShaderPath, int const vertShaderCount, int const fragShaderCount)
	{
		if(GL20Support)
			return buildShaderProgram20(vertShaderPath, fragShaderPath, vertShaderCount, fragShaderCount);
		else
			return buildShaderProgramARB(vertShaderPath, fragShaderPath, vertShaderCount, fragShaderCount);
	}
	
private:
	FILE *logFile;
	
	GLuint buildShaderProgram20(char const * const * vertShaderPath, char const * const * fragShaderPath, int const vertShaderCount, int const fragShaderCount)
	{
		#ifndef MAX_SHADERS
		#define MAX_SHADERS 10
		#endif
		if(vertShaderCount > MAX_SHADERS || fragShaderCount > MAX_SHADERS)
		{
			printf("This function can only build shaders with %i files. Exiting.\n", MAX_SHADERS);
			if(logFile != NULL)
				fprintf(logFile, "This function can only build shaders with %i files. Exiting.\n", MAX_SHADERS);
			exit(1);
		}
		
		GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		char * vertSource[MAX_SHADERS];
		char * fragSource[MAX_SHADERS];
		
		//read shader source
		for(int i=0; i<vertShaderCount; i++)
			vertSource[i] = readFile( vertShaderPath[i] );
		for(int i=0; i<fragShaderCount; i++)
			fragSource[i] = readFile( fragShaderPath[i] );
		
		glShaderSource(vertShader, vertShaderCount, (char const **)vertSource, NULL);
		glShaderSource(fragShader, fragShaderCount, (char const **)fragSource, NULL);
		
		for(int i=0; i<vertShaderCount; i++)
			free( (void*) vertSource[i]);
		for(int i=0; i<fragShaderCount; i++)
			free( (void*) fragSource[i]);
		
		glCompileShader(vertShader);
		printShaderInfoLog(vertShader, vertShaderPath, vertShaderCount);
		glCompileShader(fragShader);
		printShaderInfoLog(fragShader, fragShaderPath, fragShaderCount);
		
		GLuint prog = glCreateProgram();
		glAttachShader(prog, vertShader);
		glAttachShader(prog, fragShader);
		
		glLinkProgram(prog);
		printProgramInfoLog(prog, vertShaderPath, fragShaderPath, vertShaderCount, fragShaderCount);
		
		return prog;
	}
	
	GLuint buildShaderProgramARB(char const * const * vertShaderPath, char const * const * fragShaderPath, int const vertShaderCount, int const fragShaderCount)
	{
		#ifndef MAX_SHADERS
		#define MAX_SHADERS 10
		#endif
		if(vertShaderCount > MAX_SHADERS || fragShaderCount > MAX_SHADERS)
		{
			printf("This function can only build shaders with %i files. Exiting.\n", MAX_SHADERS);
			if(logFile != NULL)
				fprintf(logFile, "This function can only build shaders with %i files. Exiting.\n", MAX_SHADERS);
			exit(1);
		}
		
		GLhandleARB vertShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
		GLhandleARB fragShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
		char * vertSource[MAX_SHADERS];
		char * fragSource[MAX_SHADERS];
		
		//read shader source
		for(int i=0; i<vertShaderCount; i++)
			vertSource[i] = readFile( vertShaderPath[i] );
		for(int i=0; i<fragShaderCount; i++)
			fragSource[i] = readFile( fragShaderPath[i] );
		
		glShaderSourceARB(vertShader, vertShaderCount, (char const **)vertSource, NULL);
		glShaderSourceARB(fragShader, fragShaderCount, (char const **)fragSource, NULL);
		
		for(int i=0; i<vertShaderCount; i++)
			free( (void*) vertSource[i]);
		for(int i=0; i<fragShaderCount; i++)
			free( (void*) fragSource[i]);
		
		glCompileShaderARB(vertShader);
		printInfoLog(vertShader, vertShaderPath, NULL, vertShaderCount, 0);
		glCompileShaderARB(fragShader);
		printInfoLog(fragShader, NULL, fragShaderPath, 0, fragShaderCount);
		
		GLhandleARB prog = glCreateProgramObjectARB();
		glAttachObjectARB(prog, vertShader);
		glAttachObjectARB(prog, fragShader);
		
		glLinkProgramARB(prog);
		printInfoLog(prog, vertShaderPath, fragShaderPath, vertShaderCount, fragShaderCount);
		
		return prog;
	}
	
	void printShaderInfoLog(GLuint obj, char const * const * shaderPaths, int const shaderCount)
	{
		int infologLength = 0;
		int charsWritten  = 0;
		char *infoLog;
		
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
		
		if (infologLength > 0)
		{
			printf("Shader info log for:\n");
			if(logFile != NULL)
				fprintf(logFile, "Shader info log for:\n");
			for(int i=0; i<shaderCount; i++)
			{
				printf("  %s\n", shaderPaths[i]);
				if(logFile != NULL)
					fprintf(logFile, "  %s\n", shaderPaths[i]);
			}
			infoLog = (char *)malloc(infologLength);
			glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
			printf("%s\n",infoLog);
			if(logFile != NULL)
				fprintf(logFile, "%s\n",infoLog);
			free(infoLog);
		}
		else
		{
		}
	}
	
	void printProgramInfoLog(GLuint obj, char const * const * vertPaths, char const * const * fragPaths, int const vertShaderCount, int const fragShaderCount)
	{
		int infologLength = 0;
		int charsWritten  = 0;
		char *infoLog;
		
		glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
		
		if (infologLength > 0)
		{
			printf("Shader program info log for:\n");
			if(logFile != NULL)
				fprintf(logFile, "Shader program info log for:\n");
			for(int i=0; i<vertShaderCount; i++)
			{
				printf("  %s\n", vertPaths[i]);
				if(logFile != NULL)
					fprintf(logFile, "  %s\n", vertPaths[i]);
			}
			for(int i=0; i<fragShaderCount; i++)
			{
				printf("  %s\n", fragPaths[i]);
				if(logFile != NULL)
					fprintf(logFile, "  %s\n", fragPaths[i]);
			}

			infoLog = (char *)malloc(infologLength);
			glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
			printf("%s\n",infoLog);
			if(logFile != NULL)
				fprintf(logFile, "%s\n",infoLog);
			free(infoLog);
		}
	}
	
	void printInfoLog(GLhandleARB obj, char const * const * vertPaths, char const * const * fragPaths, int const vertShaderCount, int const fragShaderCount)
	{
		int infologLength = 0;
		int charsWritten  = 0;
		char *infoLog;
		
		glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,
								  &infologLength);
		
		if (infologLength > 0)
		{
			printf("Shader info log for:\n");
			if(logFile != NULL)
				fprintf(logFile, "Shader info log for:\n");
			for(int i=0; i<vertShaderCount; i++)
			{
				printf("  %s\n", vertPaths[i]);
				if(logFile != NULL)
					fprintf(logFile, "  %s\n", vertPaths[i]);
			}
			for(int i=0; i<fragShaderCount; i++)
			{
				printf("  %s\n", fragPaths[i]);
				if(logFile != NULL)
					fprintf(logFile, "  %s\n", fragPaths[i]);
			}
			infoLog = (char *)malloc(infologLength);
			glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
			printf("%s\n",infoLog);
			free(infoLog);
		}
	}
	
	FILE* sfopen(const char* name, const char *mode)
	{
		FILE *file = NULL;
		file = fopen(name, mode);
		
		if(file == NULL)
		{
			printf("Unable to open file '%s' in mode '%s'\n", name, mode);
			if(logFile != NULL)
				fprintf(logFile, "Unable to open file '%s' in mode '%s'\n", name, mode);
			exit(1);
		}
		
		return file;
	}
	
	char* readFile(char const * fileName)
	{
		FILE *file;
		
		//open file
		file = sfopen(fileName, "rb");
		
		//get size
		fseek(file, 0, SEEK_END);
		int fileDataLength = ftell(file);
		rewind(file);
		
		//get memory for the data
		char * fileData = new char[fileDataLength+1];
		fread(fileData, 1, fileDataLength, file);
		
		fclose(file);
		fileData[fileDataLength] = '\0';
		
		return fileData;
	}

};

#endif
