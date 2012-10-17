#ifndef RENDERER_H
#define RENDERER_H

class Renderer
{
private:
public:
	virtual void init()					= 0;
	virtual void renderFrame()			= 0;
};

#endif //RENDERER_H