#include "main.h"
#include "vendor/hash/md5.h"
#include "CheckFileHash.h"

bool CheckFile(mdFile* mdChkFile);

bool FileCheckSum()
{
	mdFile *mdCheckSumFile = new mdFile;

	// main.scm - 0 3212378963 3226940476 2512197760
	// script.img - 0 2761982703 3892831165 2244854639
	// vehicles.ide - 0 401823104 3800320153 1955768670
	// handling.cfg - 0 483133070 2164808380 3547226580
	// peds.ide - 0 3301990606 2094634834 1824250679

	sprintf(mdCheckSumFile->szFileLocation, "%sSAMP/main.scm", g_pszStorage);
	mdCheckSumFile->iCorrectDigestArray[0] = 0;
	mdCheckSumFile->iCorrectDigestArray[1] = 3212378963;
	mdCheckSumFile->iCorrectDigestArray[2] = 3226940476;
	mdCheckSumFile->iCorrectDigestArray[3] = 2512197760;
	if(!CheckFile(mdCheckSumFile)) return false;

	sprintf(mdCheckSumFile->szFileLocation, "%sSAMP/script.img", g_pszStorage);
	mdCheckSumFile->iCorrectDigestArray[0] = 0;
	mdCheckSumFile->iCorrectDigestArray[1] = 2761982703;
	mdCheckSumFile->iCorrectDigestArray[2] = 3892831165;
	mdCheckSumFile->iCorrectDigestArray[3] = 2244854639;
	if(!CheckFile(mdCheckSumFile)) return false;

	sprintf(mdCheckSumFile->szFileLocation, "%sSAMP/vehicles.ide", g_pszStorage);
	mdCheckSumFile->iCorrectDigestArray[0] = 0;
	mdCheckSumFile->iCorrectDigestArray[1] = 401823104;
	mdCheckSumFile->iCorrectDigestArray[2] = 3800320153;
	mdCheckSumFile->iCorrectDigestArray[3] = 1955768670;
	if(!CheckFile(mdCheckSumFile)) return false;

	sprintf(mdCheckSumFile->szFileLocation, "%sSAMP/handling.cfg", g_pszStorage);
	mdCheckSumFile->iCorrectDigestArray[0] = 0;
	mdCheckSumFile->iCorrectDigestArray[1] = 483133070;
	mdCheckSumFile->iCorrectDigestArray[2] = 2164808380;
	mdCheckSumFile->iCorrectDigestArray[3] = 3547226580;
	if(!CheckFile(mdCheckSumFile)) return false;

	sprintf(mdCheckSumFile->szFileLocation, "%sSAMP/peds.ide", g_pszStorage);
	mdCheckSumFile->iCorrectDigestArray[0] = 0;
	mdCheckSumFile->iCorrectDigestArray[1] = 3301990606;
	mdCheckSumFile->iCorrectDigestArray[2] = 2094634834;
	mdCheckSumFile->iCorrectDigestArray[3] = 1824250679;
	if(!CheckFile(mdCheckSumFile)) return false;

	return true;
}

bool CheckFile(mdFile* mdChkFile)
{
	FILE* fLocalFile;
	fLocalFile = fopen(mdChkFile->szFileLocation, "r");
	int x;
	unsigned char szBuffer[1024];

	if(!fLocalFile)
	{
		Log("Error trying to load %s", mdChkFile->szFileLocation);
		return false;
	}

	MD5_CTX mdCTX;
	MD5Init(&mdCTX);
	while((x = fread(szBuffer, 1, sizeof(szBuffer), fLocalFile) > 0)) MD5Update(&mdCTX, szBuffer, x);
	MD5Final(mdChkFile->szRawLocalDigest, &mdCTX);

	if(ferror(fLocalFile))
	{
		Log("CRC32 checksum failed: an error occurred checking \'%s\'.", mdChkFile->szFileLocation);
		fclose(fLocalFile);
		return false;
	}

	if((mdChkFile->iCorrectDigestArray[0] != mdCTX.buf[0]) || (mdChkFile->iCorrectDigestArray[1] != mdCTX.buf[1]) || (mdChkFile->iCorrectDigestArray[2] != mdCTX.buf[2]) || (mdChkFile->iCorrectDigestArray[3] != mdCTX.buf[3]))
	{
		Log("CRC32 checksum failed: \'%s\'.", mdChkFile->szFileLocation);
		fclose(fLocalFile);
		return false;
	}

	fclose(fLocalFile);
	return true;
}