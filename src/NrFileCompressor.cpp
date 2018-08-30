/********************************************************************************
 *   Copyright (C) 2018 by NetResults S.r.l. ( http://www.netresults.it )       *
 *   Author(s):                                                                 *
 *              Francesco Lamonica  <f.lamonica@netresults.it>                  *
 ********************************************************************************/

#include "NrFileCompressor.h"
#include "Zip/miniz.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

#include <iostream>

#if defined(__GNUC__)
  // Ensure we get the 64-bit variants of the CRT's file I/O calls
  #ifndef _FILE_OFFSET_BITS
    #define _FILE_OFFSET_BITS 64
  #endif
  #ifndef _LARGEFILE64_SOURCE
    #define _LARGEFILE64_SOURCE 1
  #endif
#endif


NrFileCompressor::NrFileCompressor()
{

}


static const char *s_pComment = "Zipped with NrFileCompressor!";

int NrFileCompressor::compressZipFile(const QString &filename)
{
    std::cout << "Compressing (ZIP) file " << filename.toStdString() << std::endl;

    QString destfilename = filename + ".zip";

    mz_zip_archive zip_archive;

    //reset the zip archive
    memset(&zip_archive, 0, sizeof(zip_archive));

    //init for writing
    bool res = mz_zip_writer_init_file(&zip_archive, destfilename.toLatin1().constData(), 0);

    if (!res)
    {
        std::cerr << "" << mz_zip_get_error_string(mz_zip_get_last_error(&zip_archive)) << std::endl;
        return EXIT_FAILURE;
    }

    //add file to the archive with the same internal (the one that will be unzipped it) as the original
    res = mz_zip_writer_add_file(&zip_archive, filename.toLatin1().constData(),
                                  filename.toLatin1().constData(),
                                  s_pComment, (quint16)strlen(s_pComment), MZ_DEFAULT_COMPRESSION);
    if (!res)
    {
        std::cerr << "Error while adding a zip file to zip archive: " << mz_zip_get_error_string(mz_zip_get_last_error(&zip_archive)) << std::endl;
        return EXIT_FAILURE;
    }

    res = mz_zip_writer_finalize_archive(&zip_archive);
    if (!res)
    {
        std::cerr << "Error while finalizing zip archive: " << mz_zip_get_error_string(mz_zip_get_last_error(&zip_archive)) << std::endl;
        return EXIT_FAILURE;
    }


    return 0;
}



quint8 NrFileCompressor::getByte(quint32 var, quint8 bytenum)
{
    int shift;
    switch(bytenum) {
    case 1:
        shift = 8;
        break;
    case 2:
        shift = 16;
        break;
    case 3:
        shift = 24;
        break;
    default: //(LSB)
        shift = 0;
        break;
    }
    return (var >> shift) & 0xFF;
}


int NrFileCompressor::writeGzipHeader(QFile *pFile, quint32 i_mtime)
{
    /*
        It builds the GZIP file structure with his header and footer in this format:
        (http://www.zlib.org/rfc-gzip.html)

        +---+---+---+---+---+---+---+---+---+---+=============================+---+---+---+---+---+---+---+---+
        |ID1|ID2|CM |FLG|     MTIME     |XFL|OS |   ....COMPRESSED DATA....   |     CRC32     |     ISIZE     |
        +---+---+---+---+---+---+---+---+---+---+=============================+---+---+---+---+---+---+---+---+

        The first 10 bytes header is fixed, the CRC32 is the checksum of uncompressed data
        and the ISIZE is the size of uncompressed data in bytes.
     */
    const char id1      = 31;   //Fixed value identifying GZip
    const char  id2     = 139;  //Fixed value identifying GZip
    const char  cm      = 8;    //(Fixed value identifying GZip) is the standard deflate method
    const char  flg     = 0;    //0 means no extra flags
    quint8 mtime[4];
    const char  xflg    = 0;    //0 means no extra flags

#ifdef WIN32
    const char  os      = 0;    //0 is Dos/win, 3 is Unix-style (used for line endings)
#else
    const char  os      = 3;    //0 is Dos/win, 3 is Unix-style (used for line endings)
#endif

    mtime[0] = getByte(i_mtime, 0);
    mtime[1] = getByte(i_mtime, 1);
    mtime[2] = getByte(i_mtime, 2);
    mtime[3] = getByte(i_mtime, 3);


    pFile->write(&id1, 1);
    pFile->write(&id2, 1);
    pFile->write(&cm, 1);
    pFile->write(&flg, 1);
    pFile->write((const char*) &mtime, 4);
    pFile->write(&xflg, 1);
    pFile->write(&os, 1);

    return 0;
}


int NrFileCompressor::writeGzipFooter(QFile *pFile, quint32 i_crc32, quint32 i_size)
{
    quint8 size[4];
    size[0] = getByte(i_size, 0);
    size[1] = getByte(i_size, 1);
    size[2] = getByte(i_size, 2);
    size[3] = getByte(i_size, 3);

    quint8 crc32[4];
    crc32[0] = getByte(i_crc32, 0);
    crc32[1] = getByte(i_crc32, 1);
    crc32[2] = getByte(i_crc32, 2);
    crc32[3] = getByte(i_crc32, 3);

    pFile->write((const char*) &crc32, 4);
    pFile->write((const char*) &size, 4);

    return 0;
}

int NrFileCompressor::compressGzipFile(const QString &filename)
{
    qDebug() << "Compressing (GZIP) file " << filename;
    int level = Z_BEST_COMPRESSION;
    z_stream stream;

    const qint64 BUF_SIZE = (1024 * 1024);
    quint8 s_inbuf[BUF_SIZE];
    quint8 s_outbuf[BUF_SIZE];

    // Init the z_stream
    memset(&stream, 0, sizeof(stream));
    stream.next_in = s_inbuf;
    stream.avail_in = 0;
    stream.next_out = s_outbuf;
    stream.avail_out = BUF_SIZE;

    QFile fin(filename);
    QFile fout(filename + ".gz");

    fin.open(QIODevice::ReadOnly);
    fout.open(QIODevice::WriteOnly);

    //write the GZip file header
    QFileInfo finfo(fin);
    writeGzipHeader(&fout, static_cast<quint32>(finfo.lastModified().toSecsSinceEpoch()));

    // Compression.
    qint64 finSize = fin.size();
    qint64 infile_remaining = finSize;

    //if (deflateInit(&stream, level) != Z_OK)
    if (deflateInit2(&stream, level, MZ_DEFLATED, -MZ_DEFAULT_WINDOW_BITS, 9, MZ_DEFAULT_STRATEGY) != Z_OK)
    {
        std::cerr << "deflateInit2() failed!" << std::endl;
        return EXIT_FAILURE;
    }

    //init the crc for uncompressed data
    ulong crc = mz_crc32(0, nullptr, 0);

    //reading loop
    for ( ; ; )
    {
        int status;
        if (!stream.avail_in)
        {
          // Input buffer is empty, so read more bytes from input file.
          uint n = qMin((qint64)BUF_SIZE, infile_remaining);

          if (fin.read((char*)s_inbuf, n) != n)
          {
            std::cerr << "Failed reading from input file!" << std::endl;
            return EXIT_FAILURE;
          }

          //update the crc
          crc = mz_crc32(crc, s_inbuf, n);

          stream.next_in = s_inbuf;
          stream.avail_in = n;

          infile_remaining -= n;
          std::cout << "Input bytes remaining: " << infile_remaining << std::endl;
        }

        status = deflate(&stream, infile_remaining ? Z_NO_FLUSH : Z_FINISH);

        if ((status == Z_STREAM_END) || (!stream.avail_out))
        {
          // Output buffer is full, or compression is done, so write buffer to output file.
          uint n = BUF_SIZE - stream.avail_out;
          if (fout.write((char*)s_outbuf, n) != n)
          {
            std::cerr << "Failed writing to output file!" << std::endl;
            return EXIT_FAILURE;
          }
          stream.next_out = s_outbuf;
          stream.avail_out = BUF_SIZE;
        }

        if (status == Z_STREAM_END)
            break;
        else if (status != Z_OK)
        {
            std::cerr << "deflate() failed with status: " << status << std::endl;
            return EXIT_FAILURE;
        }
    }

    if (deflateEnd(&stream) != Z_OK)
    {
        std::cerr << "deflateEnd() failed!" << std::endl;
        return EXIT_FAILURE;
    }

    //This is a fast modulo to power-of-2 numbers
    quint32 modsize = static_cast<quint32>(finSize & (LONG_MAX - 1));
    //write the GZIP file footer
    writeGzipFooter(&fout, static_cast<quint32>(crc), modsize);

    fin.close();
    fout.close();

    return Z_OK;
}

