//
// Created by monty on 08/12/16.
//

#ifndef NOUDAR_CORE_IFILELOADERDELEGATE_H
#define NOUDAR_CORE_IFILELOADERDELEGATE_H
namespace odb {

	struct StaticBuffer {
		uint8_t *data;
		size_t size;
	};

	class IFileLoaderDelegate {
	public:
		virtual const char* getFilePathPrefix() = 0;
		virtual vector<char> loadBinaryFileFromPath( const char* path ) = 0;
		virtual StaticBuffer loadFileFromPath( const char* path ) = 0;
	};
}
#endif //NOUDAR_CORE_IFILELOADERDELEGATE_H
