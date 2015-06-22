#include "TimePoint.hpp"
#include "QualitativeTimePoint.hpp"

namespace templ {
namespace solvers {
namespace temporal {
namespace point_algebra {

TimePoint::TimePoint(uint64_t lowerBound, uint64_t upperBound, Type type)
    : mLowerBound(lowerBound)
    , mUpperBound(upperBound)
    , mType(type)
{
    TimePoint::validateBounds(mLowerBound, mUpperBound);
}

TimePoint::TimePoint(uint64_t lowerBound, uint64_t upperBound)
    : mLowerBound(lowerBound)
    , mUpperBound(upperBound)
    , mType(QUANTITATIVE)

{
    TimePoint::validateBounds(mLowerBound, mUpperBound);
}

void TimePoint::validateBounds(uint64_t lowerBound, uint64_t upperBound)
{
    if(upperBound < lowerBound)
    {
        std::stringstream ss;
        ss << "TimePoint: upper bound (" << upperBound << ") smaller than lower bound (" << lowerBound << ")";
        throw std::invalid_argument(ss.str());
    }
}

bool TimePoint::equals(TimePoint::Ptr other) const
{
    if(mType != other->getType())
    {
        throw std::invalid_argument("templ::solvers::temporal::point_algebra::TimePoint::equals: cannot compare timepoints of different types");
    }

    // TODO: improve comparision integration
    if(mType == QUANTITATIVE)
    {
        return *this == *other.get();
    } else if(mType== QUALITATIVE)
    {
        return *dynamic_cast<QualitativeTimePoint*>(const_cast<TimePoint*>(this)) == *dynamic_cast<QualitativeTimePoint*>(other.get());
    } else {
        throw std::invalid_argument("templ::solvers::temporal::point_algebra::TimePoint::equals: cannot compare timepoints of unknown types");
    }
}

bool TimePoint::operator<(const TimePoint& other) const
{
    if(mLowerBound != other.mLowerBound)
    {
        return mLowerBound < other.mLowerBound;
    } else {
        return mUpperBound < other.mUpperBound;
    }
}

TimePoint::Ptr TimePoint::create(const Label& label)
{
    return TimePoint::Ptr( new QualitativeTimePoint(label) );
}

TimePoint::Ptr TimePoint::create(uint64_t lowerBound, uint64_t upperBound)
{
    return TimePoint::Ptr( new TimePoint(lowerBound, upperBound) );
}

std::string TimePoint::toString() const
{
    std::stringstream ss;
    ss << "Timepoint: [" << mLowerBound << "," << mUpperBound << "]";
    return ss.str();
}

} // end namespace point_algebra
} // end namespace temporal
} // end namespace solvers
} // end namespace templ
