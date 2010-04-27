//  This file is part of Nifti2Dicom, is an open source converter from 
//  3D NIfTI images to 2D DICOM series.
//
//  Copyright (C) 2008, 2009, 2010 Daniele E. Domenichelli
//
//  Nifti2Dicom is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Nifti2Dicom is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Nifti2Dicom.  If not, see <http://www.gnu.org/licenses/>.

// $Id$


#include "n2dInputFilter.h"

#include <itkRescaleIntensityImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkOrientImageFilter.h>
#include <sstream>
//#define NO_REORIENT


namespace n2d {


//BEGIN DICOM tags
const std::string patientorientationtag ( "0020|0020" );
//END DICOM tags


//BEGIN Default values
const std::string defaultpatientorientation ( "L\\R" );
//END Default values

template<class TPixel> bool InputFilter::CreateFilter(void)
{
//BEGIN Typedefs
    typedef itk::Image<TPixel, Dimension>      InternalImageType;
    typedef itk::OrientImageFilter<InternalImageType,InternalImageType> OrienterType;
    typedef itk::RescaleIntensityImageFilter<InternalImageType, DICOM3DImageType > RescaleType;
    typedef itk::CastImageFilter < InternalImageType, DICOM3DImageType > CastType;
//END Typedefs

//BEGIN declarations
    typename OrienterType::Pointer orienter;
    typename RescaleType::Pointer rescaleFilter;
    typename CastType::Pointer cast;
//END declarations

    typename InternalImageType::ConstPointer internalImage;
    internalImage = dynamic_cast< const InternalImageType* >(m_InputImage.GetPointer());
    if(!internalImage)
    {
	std::cerr<<"Error Null Pointer In Filter"<<std::endl;
        return false;
    } 
//BEGIN Orienting image
#ifndef NO_REORIENT
    orienter = OrienterType::New();
    orienter->UseImageDirectionOn();
    orienter->SetDesiredCoordinateOrientation(itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI); //Orient to RAI
    orienter->SetInput(internalImage);

    try
    {
        std::cout << " * \033[1;34mOrienting\033[0m... " << std::endl;
        orienter->Update();
        std::cout << " * \033[1;34mOrienting\033[0m... \033[1;32mDONE\033[0m" << std::endl;
    }
    catch ( itk::ExceptionObject & ex )
    {
        std::cout << " * \033[1;34mOrienting\033[0m... \033[1;31mFAIL\033[0m" << std::endl;
        std::string message;
        message = ex.GetLocation();
        message += "\n";
        message += ex.GetDescription();
        std::cerr << message << std::endl;
        return false;
    }

    itk::EncapsulateMetaData<std::string>(m_Dict, patientorientationtag, defaultpatientorientation);
#endif
//END Orienting image



    if (m_FiltersArgs.rescale)
    {
    //BEGIN Rescale
        rescaleFilter = RescaleType::New();
#ifdef NO_REORIENT
        rescaleFilter->SetInput(m_InputImage);
#else
        rescaleFilter->SetInput(orienter->GetOutput());
#endif
        rescaleFilter->SetOutputMinimum( 0 );
        rescaleFilter->SetOutputMaximum( (2^11)-1 );

        try
        {
            std::cout << " * \033[1;34mRescaling\033[0m... " << std::endl;
            rescaleFilter->Update();
            std::cout << " * \033[1;34mRescaling\033[0m... \033[1;32mDONE\033[0m" << std::endl;
        }
        catch ( itk::ExceptionObject & ex )
        {
            std::cout << " * \033[1;34mRescaling\033[0m... \033[1;31mFAIL\033[0m" << std::endl;
            std::string message;
            message = ex.GetLocation();
            message += "\n";
            message += ex.GetDescription();
            std::cerr << message << std::endl;
            return true;
        }

        m_FilteredImage =rescaleFilter->GetOutput();
    //END Rescale
    }
    else
    {
    //BEGIN Cast
        // Caster
        cast = CastType::New();
#ifdef NO_REORIENT
        cast->SetInput(m_InputImage);
#else
        cast->SetInput(orienter->GetOutput());
#endif
        try
        {
            std::cout << " * \033[1;34mCasting\033[0m... " << std::endl;
            cast->Update();
            std::cout << " * \033[1;34mCasting\033[0m... \033[1;32mDONE\033[0m" << std::endl;
        }
        catch ( itk::ExceptionObject & ex )
        {
            std::cout << " * \033[1;34mCasting\033[0m... \033[1;31mFAIL\033[0m" << std::endl;
            std::string message;
            message = ex.GetLocation();
            message += "\n";
            message += ex.GetDescription();
            std::cerr << message << std::endl;
            return false;
        }
        m_FilteredImage = cast->GetOutput();
    //END Cast
    }
return true;

}

bool InputFilter::Filter( void )
{
    
#ifdef DEBUG
    std::cout << "InputFilter - BEGIN" << std::endl;
    std::cout << "InputFilter::m_InputImage directions:" << std::endl;
    std::cout << m_InputImage->GetDirection() << std::endl;
#endif // DEBUG
   bool ret=false;
   std::cout<<m_InputPixelType<<std::endl;
 
   switch(m_InputPixelType)
   {
   	case itk::ImageIOBase::UCHAR:
	{	
		ret=CreateFilter<unsigned char>();
		break;
	}
   	case itk::ImageIOBase::CHAR:
	{	
		ret=CreateFilter<char>();
		break;
	}
   	case itk::ImageIOBase::USHORT:
	{	
		ret=CreateFilter<unsigned short>();
		break;
	}
   	case itk::ImageIOBase::SHORT:
	{	
		ret=CreateFilter<short>();
		break;
	}
   	case itk::ImageIOBase::UINT:
	{	
		ret=CreateFilter<unsigned int>();
		break;
	}
   	case itk::ImageIOBase::INT:
	{	
		ret=CreateFilter<int>();
		break;
	}
   	case itk::ImageIOBase::ULONG:
	{	
		ret=CreateFilter<unsigned long>();
		break;
	}
   	case itk::ImageIOBase::LONG:
	{	
		ret=CreateFilter<long>();
		break;
	}
   	case itk::ImageIOBase::FLOAT:
	{	
		ret=CreateFilter<float>();
		break;
	}
   	case itk::ImageIOBase::DOUBLE:
	{	
		ret=CreateFilter<double>();
		break;
	}
	default:
	{
		std::cerr<<"Error wrong pixel type"<<std::endl;
		return false;
	}
   }


#ifdef DEBUG
    std::cout << "InputFilter - END" << std::endl;
    std::cout << "InputFilter::m_FilteredImage directions:" << std::endl;
    std::cout << m_FilteredImage->GetDirection() << std::endl;
#endif // DEBUG


    return ret;
}


} // namespace n2d




//TODO settare correttamente BITS ALLOCATED (0028,0100)/ BITS STORED (0028,0101) / HIGH BIT (0028,0102)
// (pare impossibile con itk+gdcm)
//        // Bits Allocated
//          itk::EncapsulateMetaData<std::string>( inputDict, "0028|0100", "16");
//        // Bits Stored
//          itk::EncapsulateMetaData<std::string>( inputDict, "0028|0101", "12");
//        // High Bit
//          itk::EncapsulateMetaData<std::string>( inputDict, "0028|0102", "11");
