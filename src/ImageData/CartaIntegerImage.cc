/* This file is part of the CARTA Image Viewer: https://github.com/CARTAvis/carta-backend
   Copyright 2018, 2019, 2020, 2021 Academia Sinica Institute of Astronomy and Astrophysics (ASIAA),
   Associated Universities, Inc. (AUI) and the Inter-University Institute for Data Intensive Astronomy (IDIA)
   SPDX-License-Identifier: GPL-3.0-or-later
*/

//# CartaIntegerImage.cc : specialized casacore::ImageInterface implementation for FITS

#include "CartaIntegerImage.h"

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

CartaIntegerImage::CartaIntegerImage(const std::string& filename) : casacore::ImageInterface<float>(), _filename(filename) {
    _image.reset(new casacore::PagedImage<casacore::Int>(filename));
    setCoordinateInfo(_image->coordinates());
}

CartaIntegerImage::CartaIntegerImage(const CartaIntegerImage& other) : ImageInterface<float>(other), _filename(other._filename) {
    _image.reset(new casacore::PagedImage<casacore::Int>(other._filename));
    setCoordinateInfo(_image->coordinates());
}

CartaIntegerImage::~CartaIntegerImage() {}

// Image interface

casacore::String CartaIntegerImage::imageType() const {
    return "CartaIntegerImage";
}

casacore::String CartaIntegerImage::name(bool stripPath) const {
    return _image->name();
}

casacore::IPosition CartaIntegerImage::shape() const {
    return _image->shape();
}

casacore::Bool CartaIntegerImage::ok() const {
    return _image->ok();
}

casacore::DataType CartaIntegerImage::dataType() const {
    return casacore::DataType::TpFloat;
}

casacore::Bool CartaIntegerImage::doGetSlice(casacore::Array<float>& buffer, const casacore::Slicer& section) {
    casacore::SubImage<casacore::Int> sub_image(*_image, section);
    casacore::RO_MaskedLatticeIterator<casacore::Int> lattice_iter(sub_image);
    for (lattice_iter.reset(); !lattice_iter.atEnd(); ++lattice_iter) {
        casacore::Array<float> cursor_data = (const casacore::Array<float>&)lattice_iter.cursor(); ////
        casacore::IPosition cursor_shape(lattice_iter.cursorShape());
        casacore::IPosition cursor_position(lattice_iter.position());
        casacore::Slicer cursor_slicer(cursor_position, cursor_shape);
        buffer(cursor_slicer) = cursor_data;
    }
    return true;
}

void CartaIntegerImage::doPutSlice(
    const casacore::Array<float>& buffer, const casacore::IPosition& where, const casacore::IPosition& stride) {
    throw(casacore::AipsError("CartaIntegerImage::doPutSlice - image is not writable"));
}

const casacore::LatticeRegion* CartaIntegerImage::getRegionPtr() const {
    return nullptr;
}

casacore::ImageInterface<float>* CartaIntegerImage::cloneII() const {
    return new CartaIntegerImage(*this);
}

void CartaIntegerImage::resize(const casacore::TiledShape& newShape) {
    throw(casacore::AipsError("CartaIntegerImage::resize - image is not writable"));
}

casacore::uInt CartaIntegerImage::advisedMaxPixels() const {
    return _image->advisedMaxPixels();
}

casacore::IPosition CartaIntegerImage::doNiceCursorShape(casacore::uInt) const {
    return _image->doNiceCursorShape(_image->advisedMaxPixels());
}

casacore::Bool CartaIntegerImage::isMasked() const {
    return _image->isMasked();
}

casacore::Bool CartaIntegerImage::hasPixelMask() const {
    return _image->hasPixelMask();
}

const casacore::Lattice<bool>& CartaIntegerImage::pixelMask() const {
    return _image->pixelMask();
}

casacore::Lattice<bool>& CartaIntegerImage::pixelMask() {
    return _image->pixelMask();
}

casacore::Bool CartaIntegerImage::doGetMaskSlice(casacore::Array<bool>& buffer, const casacore::Slicer& section) {
    return _image->doGetMaskSlice(buffer, section);
}
