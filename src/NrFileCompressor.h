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
private:
    static quint8 getByte(quint32 var, quint8 bytenum);
    int writeGzipHeader(QFile *pFile, quint32 i_mtime);
    int writeGzipFooter(QFile *pFile, quint32 i_crc32, quint32 i_size);
public:
    NrFileCompressor();
    int compressZipFile(const QString &filename);
    int compressGzipFile(const QString &filename);
};

#endif // NRFILECOMPRESSOR_H
