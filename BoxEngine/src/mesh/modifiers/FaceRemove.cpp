#include "mesh/modifiers/FaceRemove.h"
#include <mesh/MeshEditing.h>
#include <miniBoxLog.h>


bool FaceRemove::Use(MeshEditing& mesh, std::size_t faceIndex)
{
    if (!mesh.RemoveFace(faceIndex))
    {
        BOX_LOG_ERROR("FaceRemove: Failed to remove face");

        return false;
    }

    BOX_LOG_INFO("Removed face " << faceIndex);

    return true;
}
