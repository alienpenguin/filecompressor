#include <QCoreApplication>
#include <QDebug>

#include <NrFileCompressor.h>

//#define COMPRESS_GZIP 1
#define COMPRESS_ZIP 1

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    NrFileCompressor fc;

#ifdef COMPRESS_GZIP
    int gres = fc.compressGzipFile("random.txt");
    if (gres==0) qDebug() << "done";
#endif

#ifdef COMPRESS_ZIP
    int zres = fc.compressZipFile("random.txt", 6);
    if (zres == 0) qDebug() << "compress done";

    zres = fc.uncompressZipFile("random.txt.zip", ".");
    if (zres == 0) qDebug() << "uncompress done";
#endif

    return 0; //a.exec();
}
