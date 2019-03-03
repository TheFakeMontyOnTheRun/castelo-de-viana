//
// Created by monty on 08/12/16.
//

#ifndef NOUDAR_CORE_IFILELOADERDELEGATE_H
#define NOUDAR_CORE_IFILELOADERDELEGATE_H
namespace odb {

	class IFileLoaderDelegate {
	public:
		virtual const char* getFilePathPrefix() = 0;
		virtual StaticBuffer loadFileFromPath( const char* path ) = 0;
	};
}
#endif //NOUDAR_CORE_IFILELOADERDELEGATE_H
