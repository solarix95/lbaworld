
#include "testliblbadata.h"
#include "hqrfile.h"
#include "binaryreader.h"
#include "binarywriter.h"
#include "lbapalette.h"

//-------------------------------------------------------------------------------------------
void TestLiblbadata::testBinWriter()
{
    BinaryWriter writer;
    QVERIFY(writer.size() == 0);
    writer.skip(3);
    QVERIFY(writer.size() == 3);
    writer.append((quint8)0);
    QVERIFY(writer.size() == 4);
    writer.append((qint8)0);
    QVERIFY(writer.size() == 5);
    writer.append((qint16)0);
    QVERIFY(writer.size() == 7);
    writer.append((quint16)0);
    QVERIFY(writer.size() == 9);
    writer.append(QByteArray(11,'\0'));
    QVERIFY(writer.size() == 20);
}

//-------------------------------------------------------------------------------------------
void TestLiblbadata::testBinReader()
{
    BinaryWriter writer;
    writer.append((qint8)1);
    writer.append((quint8)2);
    writer.append((qint16)3);
    writer.append((quint16)4);
    writer.skip(2);
    writer.append((quint16)5);

    BinaryReader reader(writer.buffer());
    QVERIFY(reader.readInt8()  == 1);
    QVERIFY(reader.readUint8() == 2);
    QVERIFY(reader.readInt16() == 3);
    QVERIFY(reader.readUint16()== 4);
    reader.skip(2);
    QVERIFY(reader.readUint16()== 5);
}

//-------------------------------------------------------------------------------------------
void TestLiblbadata::testHqrFile()
{

    // 0 bytes
    {
        HqrFile inHqr;
        inHqr.appendBlock(QByteArray("12345678",8));

        HqrFile outHqr;
        outHqr.fromBuffer(inHqr.toByteArray());

        QVERIFY(inHqr.count() == outHqr.count());
        QVERIFY(inHqr.block(0) == outHqr.block(0));
    }

    // exact 8 bytes
    {
        HqrFile inHqr;
        inHqr.appendBlock(QByteArray("12345678",8));

        HqrFile outHqr;
        outHqr.fromBuffer(inHqr.toByteArray());

        QVERIFY(inHqr.count() == outHqr.count());
        QVERIFY(inHqr.block(0) == outHqr.block(0));
    }

    // Test Random Data 0 byte - 20'000 bytes
    {
        QByteArray nextTestString;
        do {
            HqrFile inHqr;
            inHqr.appendBlock(nextTestString);

            HqrFile outHqr;
            outHqr.fromBuffer(inHqr.toByteArray());

            QVERIFY(inHqr.count() == outHqr.count());
            QVERIFY(inHqr.block(0) == outHqr.block(0));
            nextTestString += (char)(qrand() % 256);
        } while (nextTestString.size() < 500);
    }

    // Test easy compressable data 0 byte - 20'000 bytes
    {
        QByteArray nextTestString;
        do {
            HqrFile inHqr;
            inHqr.appendBlock(nextTestString);

            HqrFile outHqr;
            outHqr.fromBuffer(inHqr.toByteArray());

            QVERIFY(inHqr.count() == outHqr.count());
            QVERIFY(inHqr.block(0) == outHqr.block(0));

            nextTestString += 'X';
        } while (nextTestString.size() < 2000);
    }

    // Big File with maximum compressable data
    {
        QByteArray testString(5000000,'X');

        HqrFile inHqr;
        inHqr.appendBlock(testString);

        HqrFile outHqr;
        QByteArray compressedHqrBuffer = inHqr.toByteArray(2);
        // qDebug() << "Compressed to" << 100*(compressedHqrBuffer.size()/(double)testString.size()) << "%";
        outHqr.fromBuffer(compressedHqrBuffer);

        QVERIFY(inHqr.count() == outHqr.count());
        QVERIFY(inHqr.block(0) == outHqr.block(0));
    }

    // Big File with (compressable) random data
    {
        QByteArray testString;
        do {
           testString += (char)(qrand() % 2);
        } while (testString.size() < 2000000);  // 5Megs

        HqrFile inHqr;
        inHqr.appendBlock(testString);

        HqrFile outHqr;
        QByteArray compressedHqrBuffer = inHqr.toByteArray(2);
        // qDebug() << "Compressed to" << 100*(compressedHqrBuffer.size()/(double)testString.size()) << "%";
        outHqr.fromBuffer(compressedHqrBuffer);

        QVERIFY(inHqr.count() == outHqr.count());
        QVERIFY(inHqr.block(0) == outHqr.block(0));
    }
}

//-------------------------------------------------------------------------------------------
void TestLiblbadata::testLbaPalette()
{
    QVector<QRgb> palette(256);
    for (int i=0; i<256; i++) {
        palette[i] = QColor(i,qMin(i+1, 255), qMin(i+2, 255)).rgb();
    }

    LbaPalette in(palette);

    // Lba-Buffer
    {
        LbaPalette lbaPal;
        lbaPal.fromBuffer(in.toBuffer());
        QVERIFY(palette == lbaPal.palette());
    }

    // Gimp-Buffer
    {
        LbaPalette gimpPal;
        gimpPal.fromBuffer(in.toGimpPalette("test"));
        QVERIFY(palette == gimpPal.palette());
    }
}

//-------------------------------------------------------------------------------------------
QTEST_MAIN(TestLiblbadata)

