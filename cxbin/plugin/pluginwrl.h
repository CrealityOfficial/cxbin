#ifndef CXBIN_PLUGINWRL_1630741230197_H
#define CXBIN_PLUGINWRL_1630741230197_H
#include "cxbin/loaderimpl.h"

namespace cxbin
{
	class WrlLoader : public LoaderImpl
	{
	public:
		WrlLoader();
		virtual ~WrlLoader();

		std::string expectExtension() override;
		bool tryLoad(FILE* f, unsigned long long fileSize) override;
		bool load(FILE* f, unsigned fileSize, std::vector<trimesh::TriMesh*>& out, ccglobal::Tracer* tracer) override;
	};
}

#endif // CXBIN_PLUGINWRL_1630741230197_H