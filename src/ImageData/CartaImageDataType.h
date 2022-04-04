/* This file is part of the CARTA Image Viewer: https://github.com/CARTAvis/carta-backend
   Copyright 2018, 2019, 2020, 2021 Academia Sinica Institute of Astronomy and Astrophysics (ASIAA),
   Associated Universities, Inc. (AUI) and the Inter-University Institute for Data Intensive Astronomy (IDIA)
   SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CARTA_BACKEND_IMAGEDATA_CARTAIMAGEDATATYPE_H_
#define CARTA_BACKEND_IMAGEDATA_CARTAIMAGEDATATYPE_H_

#include <casacore/casa/Utilities/DataType.h>
#include <casacore/images/Images/ImageInfo.h>
#include <casacore/images/Images/ImageInterface.h>
#include <casacore/lattices/Lattices/TiledShape.h>

#include <fitsio.h>
#include <images/Images/PagedImage.h>

#include "Logger/Logger.h"

namespace carta {

template <class T>
class CartaImageDataType : public casacore::ImageInterface<float> {
public:
    CartaImageDataType(const std::string& filename);
    CartaImageDataType(const CartaImageDataType& other);

    ~CartaImageDataType() override;

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

protected:
    std::shared_ptr<casacore::ImageInterface<T>> _image;
    std::string _filename;
};

} // namespace carta

#include "CartaImageDataType.tcc"

#endif // CARTA_BACKEND_IMAGEDATA_CARTAIMAGEDATATYPE_H_
