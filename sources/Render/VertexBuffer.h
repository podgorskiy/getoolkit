#pragma once
#include <stdint.h>

namespace Render
{
	class VertexBuffer
	{
		VertexBuffer( const VertexBuffer& ) = delete; // non construction-copyable
		VertexBuffer& operator=( const VertexBuffer& ) = delete; // non copyable
	public:
		VertexBuffer();
		VertexBuffer(int verticesStride, int indexSize);
		~VertexBuffer();

		void FillBuffers(
			const void*	VertexArray,
			int			numOfVertices,
			int			verticesStride,
			const void*	indexArray,
			int			numOfIndices,
			int			indexSize,
			bool		dynamic = false);

		void FillVertexBuffer(
			const void*	VertexArray,
			int			numOfVertices,
			bool		dynamic = false);

		void FillVertexBuffer(
			const void*	VertexArray,
			int			numOfVertices,
			int         verticesStride,
			bool		dynamic = false);

		void FillIndexBuffer(
			const void*	indexArray,
			int			numOfIndices,
			bool		dynamic = false);

		void FillIndexBuffer(
			const void*	indexArray,
			int			numOfIndices,
			int         indexSize,
			bool		dynamic = false);

		void Bind() const;
		static void UnBind();

		void DrawElements() const;

		int GetVertexSize() const;
		int GetIndexSize() const;

	private:
		int m_stride;
		int m_numOfVertices;
		int	m_numOfIndices;
		uint32_t m_indexType;
		uint32_t m_vertexBufferHandle;
		uint32_t m_indexBufferHandle;
	};
}
