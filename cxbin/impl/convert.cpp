#include "cxbin/convert.h"
#include "trimesh2/TriMesh.h"
#include "trimesh2/endianutil.h"

namespace cxbin
{
	void release(std::vector<trimesh::TriMesh*>& models)
	{
		for (trimesh::TriMesh* model : models)
			delete model;
		models.clear();
	}

	void tess(const std::vector<trimesh::vec3>& verts, const std::vector<int>& thisface, std::vector<trimesh::ivec3>& tris)
	{
		if (thisface.size() < 3)
			return;
		if (thisface.size() == 3) {
			tris.push_back(trimesh::ivec3(thisface[0],
				thisface[1],
				thisface[2]));
			return;
		}

		if (thisface.size() == 4) {
			// Triangulate in the direction that
			// gives the shorter diagonal
			const trimesh::point& p0 = verts[thisface[0]], & p1 = verts[thisface[1]];
			const trimesh::point& p2 = verts[thisface[2]], & p3 = verts[thisface[3]];
			float d02 = trimesh::dist2(p0, p2);
			float d13 = trimesh::dist2(p1, p3);
			int i = (d02 < d13) ? 0 : 1;
			tris.push_back(trimesh::ivec3(thisface[i],
				thisface[(i + 1) % 4],
				thisface[(i + 2) % 4]));
			tris.push_back(trimesh::ivec3(thisface[i],
				thisface[(i + 2) % 4],
				thisface[(i + 3) % 4]));
			return;
		}

		// 5-gon or higher - just tesselate arbitrarily...
		for (size_t i = 2; i < thisface.size(); i++)
			tris.push_back(trimesh::ivec3(thisface[0],
				thisface[i - 1],
				thisface[i]));
	}
}