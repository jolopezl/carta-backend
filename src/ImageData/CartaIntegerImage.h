/* This file is part of the CARTA Image Viewer: https://github.com/CARTAvis/carta-backend
   Copyright 2018, 2019, 2020, 2021 Academia Sinica Institute of Astronomy and Astrophysics (ASIAA),
   Associated Universities, Inc. (AUI) and the Inter-University Institute for Data Intensive Astronomy (IDIA)
   SPDX-License-Identifier: GPL-3.0-or-later
*/

//# CartaIntegerImage.h : FITS Image class derived from casacore::ImageInterface for images not supported by casacore,
//# including compressed and Int64

#ifndef CARTA_BACKEND_IMAGEDATA_CARTAINTEGERIMAGE_H_
#define CARTA_BACKEND_IMAGEDATA_CARTAINTEGERIMAGE_H_

#include <casacore/casa/Utilities/DataType.h>
#include <casacore/images/Images/ImageInfo.h>
#include <casacore/images/Images/ImageInterface.h>
#include <casacore/lattices/Lattices/TiledShape.h>

#include <fitsio.h>
#include <images/Images/PagedImage.h>

#include "Logger/Logger.h"

namespace carta {

class CartaIntegerImage : public casacore::ImageInterface<float> {
public:
    // Construct an image from a pre-existing file.
    CartaIntegerImage(const std::string& filename);
    CartaIntegerImage(const CartaIntegerImage& other);
    ~CartaIntegerImage() override;

    // implement casacore ImageInterface
    casacore::String imageType() const override;
    casacore::String name(bool stripPath = false) const override;
    casacore::IPosition shape() const override;
    casacore::Bool ok() const override;
    casacore::DataType dataType() const override;
    casacore::Bool doGetSlice(casacore::Array<float>& buffer, const casacore::Slicer& section) override;
    void doPutSlice(const casacore::Array<float>& buffer, const casacore::IPosition& where, const casacore::IPosition& stride) override;
    const casacore::LatticeRegion* getRegionPtr() const override;
    casacore::ImageInterface<float>* cloneII() const override;
    void resize(const casacore::TiledShape& newShape) override;
    casacore::uInt advisedMaxPixels() const override;
    casacore::IPosition doNiceCursorShape(casacore::uInt maxPixels) const override;

    // implement functions in other casacore Image classes
    casacore::Bool isMasked() const override;
    casacore::Bool hasPixelMask() const override;
    const casacore::Lattice<bool>& pixelMask() const override;
    casacore::Lattice<bool>& pixelMask() override;
    casacore::Bool doGetMaskSlice(casacore::Array<bool>& buffer, const casacore::Slicer& section) override;

private:
    std::unique_ptr<casacore::PagedImage<casacore::Int>> _image;
    std::string _filename;
};

} // namespace carta

#endif // CARTA_BACKEND_IMAGEDATA_CARTAINTEGERIMAGE_H_
