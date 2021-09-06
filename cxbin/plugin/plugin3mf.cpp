#include "plugin3mf.h"

#include "lib3mf_implicit.hpp"
#include "trimesh2/TriMesh.h"

namespace cxbin
{
	_3mfLoader::_3mfLoader()
	{

	}

	_3mfLoader::~_3mfLoader()
	{

	}

	std::string _3mfLoader::expectExtension()
	{
		return "3mf";
	}

	bool _3mfLoader::tryLoad(FILE* f, unsigned fileSize)
	{
		if (fileSize == 0)
			return false;

		unsigned char* buffer = new unsigned char[fileSize];
		Lib3MF::CInputVector<Lib3MF_uint8> Ibuffer(buffer, fileSize);
		fread(buffer, 1, fileSize, f);

		Lib3MF::PWrapper wrapper = Lib3MF::CWrapper::loadLibrary();
		Lib3MF::PModel model = wrapper->CreateModel();
		Lib3MF::PReader reader = model->QueryReader("3mf");
		reader->ReadFromBuffer(Ibuffer);
		Lib3MF::PMeshObjectIterator pmeshdata = model->GetMeshObjects();

		bool have3Mf = pmeshdata->Count() > 0;
		delete []buffer;

		return have3Mf;
	}

	bool _3mfLoader::load(FILE* f, unsigned fileSize, std::vector<trimesh::TriMesh*>& out, ccglobal::Tracer* tracer)
	{
		if (fileSize == 0)
			return false;

		unsigned char* buffer = new unsigned char[fileSize];
		Lib3MF::CInputVector<Lib3MF_uint8> Ibuffer(buffer, fileSize);
		fread(buffer, 1, fileSize, f);

		Lib3MF::PWrapper wrapper = Lib3MF::CWrapper::loadLibrary();
		Lib3MF::PModel model = wrapper->CreateModel();
		Lib3MF::PReader reader = model->QueryReader("3mf");
		reader->ReadFromBuffer(Ibuffer);
		Lib3MF::PMeshObjectIterator pmeshdata = model->GetMeshObjects();

		while (pmeshdata->MoveNext())
		{
			Lib3MF::PMeshObject obj = pmeshdata->GetCurrentMeshObject();
			int faceCount = obj->GetTriangleCount();
			
			trimesh::TriMesh* mesh = new trimesh::TriMesh();
			out.push_back(mesh);

			mesh->faces.reserve(faceCount);
			mesh->vertices.reserve(3 * faceCount);
			for (int nIdx = 0; nIdx < faceCount; nIdx++)
			{
				Lib3MF::sTriangle faceIndex = obj->GetTriangle(nIdx);
				Lib3MF::sPosition point1 = obj->GetVertex(faceIndex.m_Indices[0]);
				Lib3MF::sPosition point2 = obj->GetVertex(faceIndex.m_Indices[1]);
				Lib3MF::sPosition point3 = obj->GetVertex(faceIndex.m_Indices[2]);

				int v = mesh->vertices.size();
				mesh->vertices.push_back(trimesh::point(point1.m_Coordinates[0], point1.m_Coordinates[1], point1.m_Coordinates[2]));
				mesh->vertices.push_back(trimesh::point(point2.m_Coordinates[0], point2.m_Coordinates[1], point2.m_Coordinates[2]));
				mesh->vertices.push_back(trimesh::point(point3.m_Coordinates[0], point3.m_Coordinates[1], point3.m_Coordinates[2]));
				mesh->faces.push_back(trimesh::TriMesh::Face(v, v + 1, v + 2));
			}	
		}

		delete[]buffer;
		return true;
	}
}