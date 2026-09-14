#include "mesh/modifiers/BridgeFaces.h"
#include "mesh/MeshEditing.h"
#include "miniBoxLog.h"

bool BridgeFaces::Use(MeshEditing& mesh, std::size_t faceIndexA, std::size_t faceIndexB)
{
	// ---------------------------------------------------------
		// Validate faces
		// ---------------------------------------------------------
	if (faceIndexA >= mesh.GetFaceCount() ||
		faceIndexB >= mesh.GetFaceCount())
	{
		BOX_LOG_ERROR("BridgeFaces: Invalid face index");

		return false;
	}
	// Both faces must have the same number of vertices
	/*if (mesh.GetFaceVertexCount(faceIndexA) != mesh.GetFaceVertexCount(faceIndexB))
	{
		BOX_LOG_ERROR("BridgeFaces: Faces must have the same number of vertices");
		return false;
	}*/

	return false;
}

	