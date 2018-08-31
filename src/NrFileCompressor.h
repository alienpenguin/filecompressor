/********************************************************************************
 *   Copyright (C) 2018 by NetResults S.r.l. ( http://www.netresults.it )       *
 *   Author(s):                                                                 *
 *              Francesco Lamonica  <f.lamonica@netresults.it>                  *
 ********************************************************************************/

#ifndef NRFILECOMPRESSOR_H
#define NRFILECOMPRESSOR_H

#include <QString>
class QFile;

class NrFileCompressor
{

public:
    enum compressedFileFormatEnum
    {
        GZIP_FILE,
        ZIP_FILE
    };

    enum CompressErrorType {
        E_FILE_NOT_OPEN         =  -1,
        E_FILE_NOT_WRITEABLE    =  -2,
        E_MINIZ_ERROR           =  -3,
    };

private:
    static quint8 getByte(quint32 var, quint8 bytenum);
    static int writeGzipHeader(QFile *pFile, quint32 i_mtime);
    static int writeGzipFooter(QFile *pFile, quint32 i_crc32, quint32 i_size);

public:
    NrFileCompressor();
    static int fileCompress(const QString &filename, NrFileCompressor::compressedFileFormatEnum algo, int level);
    static int compressZipFile(const QString &filename);
    static int compressGzipFile(const QString &filename);

};

#endif // NRFILECOMPRESSOR_H
