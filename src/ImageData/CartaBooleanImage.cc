/* This file is part of the CARTA Image Viewer: https://github.com/CARTAvis/carta-backend
   Copyright 2018, 2019, 2020, 2021 Academia Sinica Institute of Astronomy and Astrophysics (ASIAA),
   Associated Universities, Inc. (AUI) and the Inter-University Institute for Data Intensive Astronomy (IDIA)
   SPDX-License-Identifier: GPL-3.0-or-later
*/

//# CartaIntegerImage.cc : specialized casacore::ImageInterface implementation for FITS

#include "CartaBooleanImage.h"

#include <casacore/casa/BasicSL/Constants.h>
#include <casacore/casa/OS/File.h>
#include <casacore/casa/Quanta/UnitMap.h>
#include <casacore/coordinates/Coordinates/DirectionCoordinate.h>
#include <casacore/coordinates/Coordinates/FITSCoordinateUtil.h>
#include <casacore/coordinates/Coordinates/SpectralCoordinate.h>
#include <casacore/images/Images/ImageFITSConverter.h>
#include <casacore/lattices/Lattices/MaskedLatticeIterator.h>
#include <casacore/measures/Measures/MDirection.h>
#include <casacore/measures/Measures/Stokes.h>
#include <casacore/tables/DataMan/TiledFileAccess.h>

#include <images/Images/SubImage.h>
#include <wcslib/fitshdr.h>
#include <wcslib/wcs.h>
#include <wcslib/wcsfix.h>
#include <wcslib/wcshdr.h>
#include <wcslib/wcsmath.h>

using namespace carta;

CartaBooleanImage::CartaBooleanImage(const std::string& filename) : casacore::ImageInterface<float>(), _filename(filename) {
    _image.reset(new casacore::PagedImage<casacore::Bool>(filename));
    setCoordinateInfo(_image->coordinates());
}

CartaBooleanImage::CartaBooleanImage(const CartaBooleanImage& other) : ImageInterface<float>(other), _filename(other._filename) {
    _image.reset(new casacore::PagedImage<casacore::Bool>(other._filename));
    setCoordinateInfo(_image->coordinates());
}

CartaBooleanImage::~CartaBooleanImage() {}

// Image interface

casacore::String CartaBooleanImage::imageType() const {
    return "CartaBooleanImage";
}

casacore::String CartaBooleanImage::name(bool stripPath) const {
    return _image->name();
}

casacore::IPosition CartaBooleanImage::shape() const {
    return _image->shape();
}

casacore::Bool CartaBooleanImage::ok() const {
    return _image->ok();
}

casacore::DataType CartaBooleanImage::dataType() const {
    return casacore::DataType::TpFloat;
}

casacore::Bool CartaBooleanImage::doGetSlice(casacore::Array<float>& buffer, const casacore::Slicer& section) {
    casacore::SubImage<casacore::Bool> sub_image(*_image, section);
    casacore::RO_MaskedLatticeIterator<casacore::Bool> lattice_iter(sub_image);
    for (lattice_iter.reset(); !lattice_iter.atEnd(); ++lattice_iter) {
        casacore::Array<casacore::Bool> cursor_data = lattice_iter.cursor();
        casacore::IPosition cursor_shape(lattice_iter.cursorShape());
        casacore::IPosition cursor_position(lattice_iter.position());
        casacore::Slicer cursor_slicer(cursor_position, cursor_shape);

        // represent the bool type as the float type
        casacore::Array<float> cursor_data_float(cursor_shape);
        auto* bool_ptr = cursor_data.data();
        auto* float_ptr = cursor_data_float.data();
        for (casacore::uInt i = 0; i < cursor_data.size(); ++i) {
            if (bool_ptr[i]) {
                float_ptr[i] = 1.0;
            } else {
                float_ptr[i] = 0.0;
            }
        }

        buffer(cursor_slicer) = cursor_data_float;
    }
    return true;
}

void CartaBooleanImage::doPutSlice(
    const casacore::Array<float>& buffer, const casacore::IPosition& where, const casacore::IPosition& stride) {
    throw(casacore::AipsError("CartaBooleanImage::doPutSlice - image is not writable"));
}

const casacore::LatticeRegion* CartaBooleanImage::getRegionPtr() const {
    return nullptr;
}

casacore::ImageInterface<float>* CartaBooleanImage::cloneII() const {
    return new CartaBooleanImage(*this);
}

void CartaBooleanImage::resize(const casacore::TiledShape& newShape) {
    throw(casacore::AipsError("CartaIBooleanImage::resize - image is not writable"));
}

casacore::uInt CartaBooleanImage::advisedMaxPixels() const {
    return _image->advisedMaxPixels();
}

casacore::IPosition CartaBooleanImage::doNiceCursorShape(casacore::uInt) const {
    return _image->doNiceCursorShape(_image->advisedMaxPixels());
}

casacore::Bool CartaBooleanImage::isMasked() const {
    return _image->isMasked();
}

casacore::Bool CartaBooleanImage::hasPixelMask() const {
    return _image->hasPixelMask();
}

const casacore::Lattice<bool>& CartaBooleanImage::pixelMask() const {
    return _image->pixelMask();
}

casacore::Lattice<bool>& CartaBooleanImage::pixelMask() {
    return _image->pixelMask();
}

casacore::Bool CartaBooleanImage::doGetMaskSlice(casacore::Array<bool>& buffer, const casacore::Slicer& section) {
    return _image->doGetMaskSlice(buffer, section);
}
