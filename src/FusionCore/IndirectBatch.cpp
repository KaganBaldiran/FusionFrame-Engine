#include "IndirectBatch.hpp"

FUSIONCORE::IndirectBatch::IndirectBatch()
{
}

FUSIONCORE::IndirectBatch::IndirectBatch(std::vector<Model*>& Models)
{
}

FUSIONCORE::IndirectBatch::IndirectBatch(std::vector<std::tuple<DrawElementsIndirectCommand, Model*, Material>>&& CommandsPerModel)
{
}
