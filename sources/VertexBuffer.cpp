#include "VertexBuffer.h"
#include <assert.h>
#include <GL/gl3w.h>

using namespace Render;


VertexBuffer::VertexBuffer() :
		m_numOfVertices(-1),
		m_numOfIndices(-1),
		m_indexType(-1),
		m_vertexBufferHandle(-1),
		m_indexBufferHandle(-1)
{
}

VertexBuffer::VertexBuffer(int verticesStride, int indexSize) : m_stride(verticesStride),
		m_numOfVertices(-1),
		m_numOfIndices(-1),
		m_indexType(-1),
		m_vertexBufferHandle(-1),
		m_indexBufferHandle(-1)
{
	if (indexSize == 2)
	{
		m_indexType = GL_UNSIGNED_SHORT;
	}
	else if (indexSize == 4)
	{
		m_indexType = GL_UNSIGNED_INT;
	}
	else
	{
		assert(false);
	}
}

static void DeleteBuffer(unsigned int& handle)
{
	if (handle != (uint32_t)-1)
	{
		GLuint* b = &handle;
		glDeleteBuffers(1, b);
		handle = -1;
	}
}

VertexBuffer::~VertexBuffer()
{
	DeleteBuffer(m_vertexBufferHandle);
	DeleteBuffer(m_indexBufferHandle);
}

void VertexBuffer::FillBuffers(
		const void*	VertexArray,
		int			numOfVertices,
		int			verticesStride,
		const void*	indexArray,
		int			numOfIndices,
		int			indexSize,
		bool		dynamic)
{
	if (indexSize == 2)
	{
		m_indexType = GL_UNSIGNED_SHORT;
	}
	else if (indexSize == 4)
	{
		m_indexType = GL_UNSIGNED_INT;
	}
	else
	{
		assert(false);
	}
	m_stride = verticesStride;
	FillVertexBuffer(VertexArray, numOfVertices, dynamic);
	FillIndexBuffer(indexArray, numOfIndices);
}

void VertexBuffer::FillVertexBuffer(
		const void*	VertexArray,
		int			numOfVertices,
		bool		dynamic)
{
	DeleteBuffer(m_vertexBufferHandle);
	m_numOfVertices = numOfVertices;
	glGenBuffers(1, &m_vertexBufferHandle);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferHandle);
	int vertexBufferSize = GetVertexSize()*numOfVertices;
	glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, VertexArray, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::FillVertexBuffer(
		const void*	VertexArray,
		int			numOfVertices,
		int         verticesStride,
		bool		dynamic)
{
	m_stride = verticesStride;
	FillVertexBuffer(VertexArray, numOfVertices, dynamic);
}

void VertexBuffer::FillIndexBuffer(
	const void*	indexArray,
	int			numOfIndices,
	bool		dynamic)
{
	DeleteBuffer(m_indexBufferHandle);
	m_numOfIndices = numOfIndices;
	glGenBuffers(1, &m_indexBufferHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferHandle);
	int indexBufferSize = GetIndexSize()*numOfIndices;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, indexArray, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void VertexBuffer::FillIndexBuffer(
	const void*	indexArray,
	int			numOfIndices,
	int         indexSize,
	bool		dynamic)
{
	if (indexSize == 2)
	{
		m_indexType = GL_UNSIGNED_SHORT;
	}
	else if (indexSize == 4)
	{
		m_indexType = GL_UNSIGNED_INT;
	}
	else
	{
		assert(false);
	}
	FillIndexBuffer(indexArray, numOfIndices, dynamic);
}

void VertexBuffer::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferHandle);
}

void VertexBuffer::UnBind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void VertexBuffer::DrawElements() const
{
	glDrawElements(GL_TRIANGLES, m_numOfIndices, m_indexType, 0);
}

int VertexBuffer::GetVertexSize() const
{
	return m_stride;
}

int VertexBuffer::GetIndexSize() const
{
	switch(m_indexType)
	{
		case GL_UNSIGNED_SHORT:
			return sizeof(unsigned short);
		case GL_UNSIGNED_INT:
			return sizeof(unsigned int);
		default: return 0;
	}
}

#include <doctest.h>

TEST_CASE("[Render] VertexBuffer")
{
	SUBCASE("Basic1")
	{
		VertexBuffer vb;
		char buff[256];
		vb.FillBuffers(buff, 8, 8, buff, 64, 4, false);
		vb.Bind();
		vb.DrawElements();
		vb.UnBind();
		CHECK_EQ(vb.GetIndexSize(), 4);
		CHECK_EQ(vb.GetVertexSize(), 8);
	}
	SUBCASE("Basic2")
	{
		VertexBuffer vb;
		char buff[256];
		vb.FillBuffers(buff, 8, 8, buff, 128, 2, false);
		vb.Bind();
		vb.DrawElements();
		vb.UnBind();
		CHECK_EQ(vb.GetIndexSize(), 2);
		CHECK_EQ(vb.GetVertexSize(), 8);
	}
}
