#include <QCoreApplication>
#include <QDebug>

#include <NrFileCompressor.h>

#define COMPRESS_GZIP 1
#define COMPRESS_ZIP 1


void parseCli(int argc, char *argv[], QString& o_filenameToCompress)
{
    if (argc == 1) {
        o_filenameToCompress = QString("random.txt");
    } else {
        o_filenameToCompress = QString(argv[1]);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    NrFileCompressor fc;
    int compressionError = -1;
    int compressionLevel = 6;
    QString filenameToCompress;
    QString compressedFilename;
    QString decompressedFilename;

    parseCli(argc, argv, filenameToCompress);
    qDebug() << QString("Compressing file: %1").arg(filenameToCompress);

#ifdef COMPRESS_GZIP
    bool gres = fc.compressGzipFile(filenameToCompress, compressionLevel, compressionError);
    if (gres == true) qDebug() << "done";
#endif

#ifdef COMPRESS_ZIP
    qDebug() << QString("Compressing file: %1").arg(filenameToCompress);
    bool zres = fc.compressZipFile(filenameToCompress, compressionLevel, compressionError);
    if (zres == true) qDebug() << "compress done";

    compressedFilename = fc.getCompressedFilename(filenameToCompress, NrFileCompressor::ZIP_ARCHIVE);
    qDebug() << QString("decompressing file: %1").arg(compressedFilename);
    zres = fc.uncompressZipFile(compressedFilename, ".", compressionError);
    if (zres == true) qDebug() << "uncompress done";
#endif

    return 0;
}
