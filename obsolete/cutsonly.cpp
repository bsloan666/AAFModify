// @doc INTERNAL
// @com This file implements the module test for CAAFDefinitionObject
//=---------------------------------------------------------------------=
//
// $Id$ $Name$
//
// The contents of this file are subject to the AAF SDK Public Source
// License Agreement Version 2.0 (the "License"); You may not use this
// file except in compliance with the License.  The License is available
// in AAFSDKPSL.TXT, or you may obtain a copy of the License from the
// Advanced Media Workflow Association, Inc., or its successor.
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
// the License for the specific language governing rights and limitations
// under the License.  Refer to Section 3.3 of the License for proper use
// of this Exhibit.
//
// WARNING:  Please contact the Advanced Media Workflow Association,
// Inc., for more information about any additional licenses to
// intellectual property covering the AAF Standard that may be required
// to create and distribute AAF compliant products.
// (http://www.amwa.tv/policies).
//
// Copyright Notices:
// The Original Code of this file is Copyright 1998-2009, licensor of the
// Advanced Media Workflow Association.  All rights reserved.
//
// The Initial Developer of the Original Code of this file and the
// licensor of the Advanced Media Workflow Association is
// Avid Technology.
// All rights reserved.
//
//=---------------------------------------------------------------------=

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "AAFTypes.h"
#include "AAFResult.h"
#include "AAFDataDefs.h"
#include "AAFFileKinds.h"
#include "AAFParameterDefs.h"
// Include the AAF interface declarations.
#include "AAF.h"

static void     LogError(HRESULT errcode, int line, const char *file)
{
  printf("Error '%0x' returned at line %d in %s\n", errcode, line, file);
}

static HRESULT moduleErrorTmp = S_OK; /* note usage in macro */
#define check(a)  \
{ moduleErrorTmp = a; \
	if (!SUCCEEDED(moduleErrorTmp)) \
	{ \
	    LogError(moduleErrorTmp, __LINE__, __FILE__);\
		goto cleanup; \
	} \
}

#define checkFatal(a)  \
{ moduleErrorTmp = a; \
  if (!SUCCEEDED(moduleErrorTmp)) \
     exit(1);\
}

static void MobIDtoString(aafMobID_constref uid, char *buf)
{
    sprintf( buf, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x-" \
		  "%02x-%02x-%02x-%02x-" \
		  "%08x%04x%04x" \
		  "%02x%02x%02x%02x%02x%02x%02x%02x",

	(int)uid.SMPTELabel[0], (int)uid.SMPTELabel[1], 
	(int)uid.SMPTELabel[2], (int)uid.SMPTELabel[3],
	(int)uid.SMPTELabel[4], (int)uid.SMPTELabel[5], 
	(int)uid.SMPTELabel[6], (int)uid.SMPTELabel[7],
	(int)uid.SMPTELabel[8], (int)uid.SMPTELabel[9], 
	(int)uid.SMPTELabel[10], (int)uid.SMPTELabel[11],

	(int)uid.length, (int)uid.instanceHigh, 
	(int)uid.instanceMid, (int)uid.instanceLow,

	uid.material.Data1, uid.material.Data2, uid.material.Data3,

	(int)uid.material.Data4[0], (int)uid.material.Data4[1], 
	(int)uid.material.Data4[2], (int)uid.material.Data4[3],
	(int)uid.material.Data4[4], (int)uid.material.Data4[5], 
	(int)uid.material.Data4[6], (int)uid.material.Data4[7] );
}

static HRESULT convert(char* cName, size_t length, const wchar_t* name)
{

  size_t status = wcstombs(cName, name, length);
  if (status == (size_t)-1) {
    fprintf(stderr, ": Error : Conversion failed.\n\n");
    return -1; 
  }
  else
  	return S_OK;
}


static HRESULT ReadAAFFile(const aafWChar * pFileName)
{
	IAAFFile *					pFile = NULL;
	IAAFHeader *				pHeader = NULL;
	IAAFDictionary*			    pDictionary = NULL;
	IEnumAAFMobs*				pMobIter = NULL;
	IAAFMob*					pMob = NULL;
	IAAFMob*					pReferencedMob = NULL;
	IEnumAAFMobSlots*			pSlotIter = NULL;
	IAAFMobSlot*				pSlot = NULL;
	IAAFSegment*				pSegment = NULL;
	IAAFSourceClip*				pSourceClip = NULL;
    IAAFOperationGroup*		    pEffect = NULL;
	IAAFSequence*				pSequence = NULL;
	IAAFComponent*				pComponent = NULL;
	IEnumAAFComponents*			pCompIter = NULL;
	IEnumAAFLocators*			pLocEnum = NULL;
	IAAFLocator*				pLocator = NULL;
	IAAFFiller*					pFiller = NULL;
	IAAFEssenceDescriptor*		pEdesc = NULL;
	IAAFSourceMob*				pSourceMob = NULL;

//	IAAFParameterDef*		    pParameterDef = NULL;
	IEnumAAFParameterDefs		*pParamDefIter = NULL;
    IAAFParameterDef		*pParamDef = NULL;

	IEnumAAFParameters		*pParamIter = NULL;
    IAAFParameter		*pParam = NULL;


	IAAFParameter*			    pParameter = NULL;
    IAAFMetaDefinition *pMetaDefinition = NULL;
	IAAFTypeDef*		pTypeDef = NULL;

	IAAFFindSourceInfo*			info = NULL;
	IAAFMasterMob*				pMasterMob = NULL;
	aafSearchCrit_t				criteria;
	aafNumSlots_t				numMobs, numSlots;
	aafNumSlots_t				numCompMobs;
	aafNumSlots_t				numTapeMobs, numFileMobs, numMasterMobs;
	HRESULT						hr = AAFRESULT_SUCCESS;
	aafMobID_t					mobID;
	aafWChar					bufW[1204];

	aafUInt32				    textSize;
	aafWChar*				    pwDesc = NULL;
	aafWChar*				    pwName = NULL;

	char*					    pszName = NULL;
	char*					    pszDesc = NULL;

	char						bufA[FILENAME_MAX];
	aafLength_t					length;

	  
	check(AAFFileOpenExistingRead ( pFileName, 0, &pFile));
	check(pFile->GetHeader(&pHeader));

    // Get the AAF Dictionary so that we can create valid AAF objects.
    check(pHeader->GetDictionary(&pDictionary));


	// Here we check on the number of tape mobs and file mobs in the file
	// If this was an importing application, then the file and tape mobs
	// often get converted first so that the compositions come in without
	// forward references.
	// Get the total number of mobs in the file (should be four)
	check(pHeader->CountMobs(kAAFAllMob, &numMobs));
	printf("***number of mobs in the file: %d\n",numMobs);

	printf("--------\n");
	// Get the number of tape mobs in the file (should be one)
	check(pHeader->CountMobs(kAAFTapeMob, &numTapeMobs));
    printf("Found %d Tape Mobs\n", numTapeMobs);
    criteria.searchTag = kAAFByMobKind;
    criteria.tags.mobKind = kAAFTapeMob;
    check(pHeader->GetMobs(&criteria, &pMobIter));
    while(AAFRESULT_SUCCESS == pMobIter->NextOne(&pMob))
    {
        check(pMob->GetMobID (&mobID));
        check(pMob->GetName (bufW, sizeof(bufW)));
        convert(bufA, sizeof(bufA), bufW);
        printf("TapeName = '%s'\n", bufA);
//        MobIDtoString(mobID, bufA);
//        printf("        (mobID %s)\n", bufA);

        pMob->Release();
        pMob = NULL;
    }

    pMobIter->Release();
    pMobIter = NULL;


	printf("--------\n");
	// Get the number of file mobs in the file (should be one)
	check(pHeader->CountMobs(kAAFFileMob, &numFileMobs));
    printf("Found %d File Mobs\n", numFileMobs);
    criteria.searchTag = kAAFByMobKind;
    criteria.tags.mobKind = kAAFFileMob;
    check(pHeader->GetMobs(&criteria, &pMobIter));
    while(AAFRESULT_SUCCESS == pMobIter->NextOne(&pMob))
    {
        check(pMob->GetMobID (&mobID));
//        MobIDtoString(mobID, bufA);
//        printf(" (mobID %s)\n", bufA);

        check(pMob->QueryInterface (IID_IAAFSourceMob, (void **)&pSourceMob));
        check(pSourceMob->GetEssenceDescriptor (&pEdesc));
        check(pEdesc->GetLocators(&pLocEnum));

        // This should read the one real locator
        if(pLocEnum->NextOne(&pLocator) == AAFRESULT_SUCCESS)
        {
//            convert(bufA, sizeof(bufA), bufW);

            pLocator->GetPath (bufW, sizeof(bufW));
            convert(bufA, sizeof(bufA), bufW);
            printf("  locator: '%s'\n", bufA);
//            printf("        There is one locator pointing to '%s'\n", bufA);


            pLocator->Release();
            pLocator = NULL;
        }
        else
            printf("  There are no locators on this file mob.\n");


        pLocEnum->Release();
        pLocEnum = NULL;

        pEdesc->Release();
        pEdesc = NULL;

        pSourceMob->Release();
        pSourceMob = NULL;

        pMob->Release();
        pMob = NULL;
    }

    pMobIter->Release();
    pMobIter = NULL;

	printf("--------\n");
	// Get the number of master mobs in the file (should be one)
	check(pHeader->CountMobs(kAAFMasterMob, &numMasterMobs));
    printf("Found %d Master Mobs\n", numMasterMobs);
    criteria.searchTag = kAAFByMobKind;
    criteria.tags.mobKind = kAAFMasterMob;
    check(pHeader->GetMobs(&criteria, &pMobIter));
    while(AAFRESULT_SUCCESS == pMobIter->NextOne(&pMob))
    {
        check(pMob->GetMobID (&mobID));
        check(pMob->GetName (bufW, sizeof(bufW)));
//        check(convert(bufA, sizeof(bufA), bufW));
        convert(bufA, sizeof(bufA), bufW);
        printf(" MasterMob Name = '%s'\n", bufA);
//        MobIDtoString(mobID, bufA);
//        printf("        (mobID %s)\n", bufA);


        pMob->Release();
        pMob = NULL;
    }

    pMobIter->Release();
    pMobIter = NULL;

	printf("--------\n");
	// Get the number of composition mobs in the file (should be one)
	check(pHeader->CountMobs(kAAFCompMob, &numCompMobs));
    printf("Found %d Composition Mobs\n", numCompMobs);

    // Enumerate over all Composition Mobs
    criteria.searchTag = kAAFByMobKind;
    criteria.tags.mobKind = kAAFCompMob;
    check(pHeader->GetMobs(&criteria, &pMobIter));
    while (pMobIter && AAFRESULT_SUCCESS == pMobIter->NextOne(&pMob))
    {
        check(pMob->GetMobID (&mobID));
//        MobIDtoString(mobID, bufA);
//        printf("    (mobID %s)\n", bufA);
        pMob->CountSlots(&numSlots);

        check(pMob->GetSlots(&pSlotIter));
        while (pSlotIter && AAFRESULT_SUCCESS == pSlotIter->NextOne(&pSlot))
        {
            check(pSlot->GetSegment(&pSegment));

            // Get the length of the segment: access through the component interface.
            check(pSegment->QueryInterface(IID_IAAFComponent, (void **) &pComponent));
            pComponent->GetLength (&length);
//            check(pComponent->GetLength (&length));
            pComponent->Release();
            pComponent = NULL;

            hr = pSegment->QueryInterface(IID_IAAFSourceClip, (void **) &pSourceClip);
            if(AAFRESULT_SUCCESS == hr)
            {
                printf("    Found source clip on slot\n");
                printf("        It has length %" AAFFMT64 "d\n", length);

                hr = pSourceClip->ResolveRef(&pReferencedMob);
                if(hr == AAFRESULT_SUCCESS)
                {
                    check(pReferencedMob->GetMobID(&mobID));
                    check(pReferencedMob->GetName (bufW, sizeof(bufW)));
                    check(convert(bufA, sizeof(bufA), bufW));
                    printf("    References mob = '%s'\n", bufA);
//                    MobIDtoString(mobID, bufA);
//                    printf("            (mobID %s)\n", bufA);

                    pReferencedMob->Release();
                    pReferencedMob = NULL;
                }


                pSourceClip->Release();
                pSourceClip = NULL;
            }
            else
            {
                hr = pSegment->QueryInterface(IID_IAAFSequence, (void **) &pSequence);
                if(AAFRESULT_SUCCESS == hr)
                {
                    aafUInt32	numComponents, item = 0;

                    check(pSequence->CountComponents (&numComponents));
                    printf("    Found Sequence on slot with %d components\n",
                        numComponents);
                    printf("        It has length %" AAFFMT64 "d\n", length);
                    check(pSequence->GetComponents (&pCompIter));
                    while (pCompIter && AAFRESULT_SUCCESS == pCompIter->NextOne(&pComponent))
                    {
                        item++;
//                        check(pComponent->GetLength (&length));
                        pComponent->GetLength (&length);
                        hr = pComponent->QueryInterface(IID_IAAFOperationGroup, (void **)&pEffect);
                        if(AAFRESULT_SUCCESS == hr)
                        {
                            IAAFOperationDef	*pEffectDef = NULL;
                            IAAFDefObject		*pDefObject = NULL;
                            aafUID_t			effectDefAUID;
//                            aafUID_t			kAAFParameterDef_ScaleX;


                            printf("        EFFECT:\n");
                            pEffect->GetOperationDefinition(&pEffectDef);
                            pEffectDef->QueryInterface(IID_IAAFDefObject, (void **) &pDefObject);
//                            pDefObject->GetAUID(&effectDefAUID);

                            pDefObject->GetNameBufLen(&textSize);
                            pwName = new wchar_t[textSize/sizeof(wchar_t)];
                            pDefObject->GetName(pwName, textSize);
                            pszName = new char[textSize/sizeof(wchar_t)];
                            wcstombs(pszName, pwName, textSize/sizeof(wchar_t));

                            pDefObject->GetDescriptionBufLen(&textSize);
                            pwDesc = new wchar_t[textSize/sizeof(wchar_t)];
                            pDefObject->GetDescription(pwDesc, textSize);
                            pszDesc = new char[textSize/sizeof(wchar_t)];
                            wcstombs(pszDesc, pwDesc, textSize/sizeof(wchar_t));

                            printf("          Name: %s\n", pszName);
//                            printf("          Desc: %s\n", pszDesc);

                            pEffectDef->GetParameterDefs(&pParamDefIter);
                            while (AAFRESULT_SUCCESS == pParamDefIter->NextOne(&pParamDef)){
                                pParamDef->GetTypeDefinition(&pTypeDef);
                                pTypeDef->QueryInterface(IID_IAAFMetaDefinition, (void **) &pMetaDefinition);
                                pMetaDefinition->GetNameBufLen(&textSize);
                                pwName = new wchar_t[textSize/sizeof(wchar_t)];
                                pMetaDefinition->GetName(pwName, textSize);
                                pszName = new char[textSize/sizeof(wchar_t)];
                                wcstombs(pszName, pwName, textSize/sizeof(wchar_t));
                                printf("            Param Name: %s\n", pszName);
                                pMetaDefinition->Release();
                                pMetaDefinition = NULL;
                                pTypeDef->Release();
                                pTypeDef = NULL;
                            }

                            pEffectDef->Release();
                            pDefObject->Release();

                            pEffect->GetParameters(&pParamIter);
                            while (AAFRESULT_SUCCESS == pParamIter->NextOne(&pParam)){
                                pParam->GetTypeDefinition(&pTypeDef);
                                pTypeDef->QueryInterface(IID_IAAFMetaDefinition, (void **) &pMetaDefinition);
                                pMetaDefinition->GetNameBufLen(&textSize);
                                pwName = new wchar_t[textSize/sizeof(wchar_t)];
                                pMetaDefinition->GetName(pwName, textSize);
                                pszName = new char[textSize/sizeof(wchar_t)];
                                wcstombs(pszName, pwName, textSize/sizeof(wchar_t));
                                printf("             Params Name: %s\n", pszName);
                                pMetaDefinition->Release();
                                pMetaDefinition = NULL;
                                pTypeDef->Release();
                                pTypeDef = NULL;
                            }
                            if(pEffect->LookupParameter(kAAFParameterDef_ScaleX, &pParameter) == AAFRESULT_SUCCESS)
                            {
                                printf("             scale\n");
                                wchar_t                         checkName[256];
                                pParameter->GetTypeDefinition(&pTypeDef);
                                pParamDef->GetTypeDefinition(&pTypeDef);
                                pTypeDef->QueryInterface(IID_IAAFMetaDefinition, (void **) &pMetaDefinition);
                                pParamDef->GetDisplayUnits (checkName, sizeof(checkName));
                                pszName = new char[sizeof(checkName)/sizeof(wchar_t)];
                                wcstombs(pszName, pwName, sizeof(checkName)/sizeof(wchar_t));
                                printf("              displayunit: %s\n", pszName);

                                pMetaDefinition->GetNameBufLen(&textSize);
                                pwName = new wchar_t[textSize/sizeof(wchar_t)];
                                pMetaDefinition->GetName(pwName, textSize);
                                pszName = new char[textSize/sizeof(wchar_t)];
                                wcstombs(pszName, pwName, textSize/sizeof(wchar_t));
                                printf("              Parameter Name: %s\n", pszName);
                                pMetaDefinition->Release();
                                pMetaDefinition = NULL;
                                pTypeDef->Release();
                                pTypeDef = NULL;
                            }

//                            aafUInt32			srcValueLen, lenRead;
//                            rc = pConstantValue->GetValueBufLen(&srcValueLen);
//                            //				Assert(srcValueLen <= sizeof(MCEffectID));
//                            rc = pConstantValue->GetValue(srcValueLen, (unsigned char*)MCEffectID, &lenRead);
//

                        }

                        hr = pComponent->QueryInterface(IID_IAAFSourceClip, (void **) &pSourceClip);
                        if(AAFRESULT_SUCCESS == hr)
                        {
                            aafSourceRef_t		ref;

                            check(pSourceClip->GetSourceReference (&ref));
                            printf("        %d) A length %" AAFFMT64 "d source clip\n", item, length);
                            check(pSourceClip->ResolveRef(&pReferencedMob));
                            check(pReferencedMob->GetMobID(&mobID));
                            check(pReferencedMob->GetName (bufW, sizeof(bufW)));
//                            check(convert(bufA, sizeof(bufA), bufW));
                            convert(bufA, sizeof(bufA), bufW);
                            printf("            References mob = '%s'\n", bufA);
//                            MobIDtoString(mobID, bufA);
//                            printf("                (mobID %s)\n", bufA);

                            hr = pReferencedMob->QueryInterface(IID_IAAFMasterMob, (void **) &pMasterMob);
                            if(AAFRESULT_SUCCESS == hr)
                            {
//                                check(pMasterMob->GetTapeName (ref.sourceSlotID,
//                                    bufW, sizeof(bufW)));
                                pMasterMob->GetTapeName (ref.sourceSlotID, bufW, sizeof(bufW));
//                                check(convert(bufA, sizeof(bufA), bufW));
                                convert(bufA, sizeof(bufA), bufW);
                                printf("            Derived from tape = '%s'\n", bufA);


                                pMasterMob->Release();
                                pMasterMob = NULL;
                            }

                            pReferencedMob->Release();
                            pReferencedMob = NULL;

                            pSourceClip->Release();
                            pSourceClip = NULL;
                        }
                        hr = pComponent->QueryInterface(IID_IAAFFiller, (void **) &pFiller);
                        if(AAFRESULT_SUCCESS == hr)
                        {
                            printf("        %d) A length %" AAFFMT64 "d filler\n", item, length);

                            pFiller->Release();
                            pFiller = NULL;
                        }

                        pComponent->Release();
                        pComponent = NULL;
                    }

                    pCompIter->Release();
                    pCompIter = NULL;

                    pSequence->Release();
                    pSequence = NULL;
                }
                else
                {
                    printf("    Found unknown segment on slot\n");
                }
            }

            pSegment->Release();
            pSegment = NULL;

            pSlot->Release();
            pSlot = NULL;
        }

        pSlotIter->Release();
        pSlotIter = NULL;


        pMob->Release();
        pMob = NULL;
    }

    pMobIter->Release();
    pMobIter = NULL;



cleanup:
	// Cleanup and return
	if (info)
		info->Release();

	if (pMasterMob)
		pMasterMob->Release();


	if(pSourceMob)
		pSourceMob->Release();

	if(pEdesc)
		pEdesc->Release();
	
	if(pLocator)
		pLocator->Release();
	
	if(pLocEnum)
		pLocEnum->Release();
	
	if (pFiller)
		pFiller->Release();

	if (pCompIter)
		pCompIter->Release();

	if (pComponent)
		pComponent->Release();

	if (pSequence)
		pSequence->Release();

	if (pReferencedMob)
		pReferencedMob->Release();

	if (pSourceClip)
		pSourceClip->Release();

	if (pSegment)
		pSegment->Release();

	if (pSlot)
		pSlot->Release();

	if (pSlotIter)
		pSlotIter->Release();

	if (pMob)
		pMob->Release();

	if (pMobIter)
		pMobIter->Release();

	if (pDictionary)
		pDictionary->Release();

	if (pHeader)
		pHeader->Release();

	if (pFile) 
	{
		pFile->Close();
		pFile->Release();
	}

	return moduleErrorTmp;
}

int main(int argc, char *argv[])
{

  aafWChar pwFileName[256];
  const char * pFileName = argv[1];

  printf("opening file %s\n", pFileName);

  mbstowcs(pwFileName, pFileName, strlen(pFileName));
  ReadAAFFile(pwFileName);

  printf("Done\n");


  return(0);
}



















