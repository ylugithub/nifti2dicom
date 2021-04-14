//  This file is part of Nifti2Dicom, is an open source converter from 
//  3D NIfTI images to 2D DICOM series.
//
//  Copyright (C) 2008, 2009, 2010 Daniele E. Domenichelli <ddomenichelli@drdanz.it>
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


#include "n2dInputFilter.h"

#include <itkRescaleIntensityImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkOrientImageFilter.h>
#include <sstream>

//original code
//#define NO_REORIENT


namespace n2d {


//BEGIN DICOM tags
const std::string patientorientationtag ( "0020|0020" );
//END DICOM tags


//BEGIN Default values
const std::string defaultpatientorientation ( "L\\R" );
//END Default values


bool InputFilter::Filter( void )
{

#ifdef DEBUG
    std::cout << "InputFilter - BEGIN" << std::endl;
    std::cout << "InputFilter::m_InputImage directions:" << std::endl;
    std::cout << m_InputImage->GetDirection() << std::endl;
#endif // DEBUG

    bool ret=false;

    switch(m_InputPixelType)
    {
        case itk::ImageIOBase::UCHAR:
        {
            ret=InternalFilter<unsigned char>();
            break;
        }
        case itk::ImageIOBase::CHAR:
        {
            ret=InternalFilter<char>();
            break;
        }
        case itk::ImageIOBase::USHORT:
        {
            ret=InternalFilter<unsigned short>();
            break;
        }
        case itk::ImageIOBase::SHORT:
        {
            ret=InternalFilter<short>();
            break;
        }
        case itk::ImageIOBase::UINT:
        {
            ret=InternalFilter<unsigned int>();
            break;
        }
        case itk::ImageIOBase::INT:
        {
            ret=InternalFilter<int>();
            break;
        }
        case itk::ImageIOBase::ULONG:
        {
            ret=InternalFilter<unsigned long>();
            break;
        }
        case itk::ImageIOBase::LONG:
        {
            ret=InternalFilter<long>();
            break;
        }
        case itk::ImageIOBase::FLOAT:
        {
            ret=InternalFilter<float>();
            break;
        }
        case itk::ImageIOBase::DOUBLE:
        {
            ret=InternalFilter<double>();
            break;
        }
        default:
        {
            std::cerr<<"ERROR: Unknown pixel type"<<std::endl;
            return false;
        }
   }

#ifdef DEBUG
    std::cout << "InputFilter - END" << std::endl;
    std::cout << "InputFilter::m_FilteredImage directions:" << std::endl;
    std::cout << m_FilteredImage->GetDirection() << std::endl;
#endif // DEBUG

    ///////////////////////////
    //Khan lab
    //date: 2021.04.13
    //validate reorient
    //48 canonical orientations, note: NO_REORIENT is the default value without specify --reorient
    std::vector<std::string> v = {"NO_REORIENT", "RIP","LIP","RSP","LSP","RIA","LIA","RSA",\
        "LSA","IRP","ILP","SRP","SLP","IRA","ILA","SRA","SLA","RPI","LPI","RAI",\
        "LAI","RPS","LPS","RAS","LAS","PRI","PLI","ARI","ALI","PRS","PLS","ARS",\
        "ALS","IPR","SPR","IAR","SAR","IPL","SPL","IAL","SAL","PIR","PSR","AIR",\
        "ASR","PIL","PSL","AIL","ASL" };

    if (std::find(v.begin(), v.end(), m_FiltersArgs.reorient) == v.end())
    {
        //--reorient type not valid.
        std::cerr << "ERROR: Unknown reorient type" << std::endl;
        return false;
    }
    return ret;
}


template<class TPixel> bool InputFilter::InternalFilter(void)
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
    // 
    // Khan lab
    // date: 2021.04.13
    // 
    std::map<std::string, itk::SpatialOrientation::ValidCoordinateOrientationFlags> string_to_orient_map = {
        { "RIP", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIP},
        { "LIP", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LIP},
        { "RSP", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RSP},
        { "LSP", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LSP},
        { "RIA", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIA},
        { "LIA", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LIA},
        { "RSA", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RSA},
        { "LSA", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LSA},
        { "IRP", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IRP},
        { "ILP", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ILP},
        { "SRP", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SRP},
        { "SLP", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SLP},
        { "IRA", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IRA},
        { "ILA", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ILA},
        { "SRA", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SRA},
        { "SLA", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SLA},
        { "RPI", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RPI},
        { "LPI", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LPI},
        { "RAI", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI},
        { "LAI", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LAI},
        { "RPS", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RPS},
        { "LPS", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LPS},
        { "RAS", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAS},
        { "LAS", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LAS},
        { "PRI", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PRI},
        { "PLI", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PLI},
        { "ARI", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ARI},
        { "ALI", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ALI},
        { "PRS", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PRS},
        { "PLS", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PLS},
        { "ARS", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ARS},
        { "ALS", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ALS},
        { "IPR", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IPR},
        { "SPR", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SPR},
        { "IAR", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IAR},
        { "SAR", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SAR},
        { "IPL", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IPL},
        { "SPL", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SPL},
        { "IAL", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IAL},
        { "SAL", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SAL},
        { "PIR", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PIR},
        { "PSR", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PSR},
        { "AIR", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_AIR},
        { "ASR", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ASR},
        { "PIL", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PIL},
        { "PSL", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PSL},
        { "AIL", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_AIL},
        { "ASL", itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ASL}
    };

    //original code
    //#ifndef NO_REORIENT
    if (m_FiltersArgs.reorient != std::string("NO_REORIENT"))
    {

        orienter = OrienterType::New();
        orienter->UseImageDirectionOn();
        //original code
        //orienter->SetDesiredCoordinateOrientation(itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI); //Orient to RAI
        
        orienter->SetDesiredCoordinateOrientation(string_to_orient_map[m_FiltersArgs.reorient]);
        orienter->SetInput(internalImage);

        try
        {
            std::cout << " * \033[1;34mOrienting\033[0m... " << std::endl;
            orienter->Update();
            std::cout << " * \033[1;34mOrienting\033[0m... \033[1;32mDONE\033[0m" << std::endl;
        }
        catch (itk::ExceptionObject& ex)
        {
            std::cout << " * \033[1;34mOrienting\033[0m... \033[1;31mFAIL\033[0m" << std::endl;
            std::string message;
            message = ex.GetLocation();
            message += "\n";
            message += ex.GetDescription();
            std::cerr << message << std::endl;
            return false;
        }

        // Khan lab: commented this line
        // date: 2021.04.13
        // reason: Patient Orientation (0020,0020) is Required if the value of Spatial Locations Preserved(0028, 135A) is REORIENTED_ONLY.
        //         Spatial Locations Preserved(0028, 135A) is type 3: most ot time, it's ignored

        //itk::EncapsulateMetaData<std::string>(m_Dict, patientorientationtag, defaultpatientorientation);
        //#endif
    }
    //END Orienting image



    if (m_FiltersArgs.rescale)
    {
        //BEGIN Rescale
        rescaleFilter = RescaleType::New();

        //original code
        //#ifdef NO_REORIENT

        if (m_FiltersArgs.reorient == "NO_REORIENT")
        {
            rescaleFilter->SetInput(internalImage);
        }
        //#else
        else
        {
            rescaleFilter->SetInput(orienter->GetOutput());
        }
        //#endif

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

        //original code
        //#ifdef NO_REORIENT

        if (m_FiltersArgs.reorient == "NO_REORIENT")
        {
            cast->SetInput(internalImage);
        }
        //#else
        else
        {
            cast->SetInput(orienter->GetOutput());
        }
        //#endif

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



} // namespace n2d




//TODO set correctly BITS ALLOCATED (0028,0100)/ BITS STORED (0028,0101) / HIGH BIT (0028,0102)
// (at the moment it looks impossible to do this using itk+gdcm)
//        // Bits Allocated
//          itk::EncapsulateMetaData<std::string>( inputDict, "0028|0100", "16");
//        // Bits Stored
//          itk::EncapsulateMetaData<std::string>( inputDict, "0028|0101", "12");
//        // High Bit
//          itk::EncapsulateMetaData<std::string>( inputDict, "0028|0102", "11");

