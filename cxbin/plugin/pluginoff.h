#ifndef CXBIN_PLUGINOFF_1630741230195_H
#define CXBIN_PLUGINOFF_1630741230195_H
#include "cxbin/loaderimpl.h"

namespace cxbin
{
	class OffLoader : public LoaderImpl
	{
	public:
		OffLoader();
		virtual ~OffLoader();

		std::string expectExtension() override;
		bool tryLoad(FILE* f, unsigned long long fileSize) override;
		bool load(FILE* f, unsigned fileSize, std::vector<trimesh::TriMesh*>& out, ccglobal::Tracer* tracer) override;
	};
}

#endif // CXBIN_PLUGINOFF_1630741230195_H