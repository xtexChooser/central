package quaedam.projection.swarm.ai

import net.minecraft.core.BlockPos
import net.minecraft.world.entity.ai.navigation.GroundPathNavigation
import net.minecraft.world.level.Level
import net.minecraft.world.level.pathfinder.Path
import quaedam.misc.causality.CausalityAnchor
import quaedam.projection.swarm.ProjectedPersonEntity
import quaedam.projection.swarm.SwarmProjection
import quaedam.projector.Projector

class ProjectedPersonNavigation(val entity: ProjectedPersonEntity, level: Level) : GroundPathNavigation(entity, level) {

    override fun createPath(set: MutableSet<BlockPos>, i: Int, bl: Boolean, j: Int, f: Float): Path? {
        if (set.any {
                Projector.findNearbyProjections(level, it, SwarmProjection.effect.get())
                    .isEmpty() && !CausalityAnchor.checkEffect(level, it)
            }) {
            return null
        }
        return super.createPath(set, i, bl, j, f)
    }

}