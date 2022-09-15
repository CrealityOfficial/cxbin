#ifndef CXBIN_PLUGINDAEDOM_1630741230198_H
#define CXBIN_PLUGINDAEDOM_1630741230198_H
#include "cxbin/loaderimpl.h"

namespace cxbin
{
	class DaeDomLoader : public LoaderImpl
	{
	public:
		DaeDomLoader();
		virtual ~DaeDomLoader();

		std::string expectExtension() override;
		bool tryLoad(FILE* f, size_t fileSize) override;
		bool load(FILE* f, size_t fileSize, std::vector<trimesh::TriMesh*>& out, ccglobal::Tracer* tracer) override;
        std::vector<std::string> associateFileList(FILE* f, ccglobal::Tracer* tracer) override;
		std::vector<float> split(std::string str, std::string pattern);
	};
}

#endif // CXBIN_PLUGINDAEDOM_1630741230198_H
