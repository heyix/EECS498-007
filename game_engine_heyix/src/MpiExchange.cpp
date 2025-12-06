// MpiExchange.cpp
#include "MpiExchange.h"
#include <numeric>

namespace FlatPhysics {

    void ExchangeGhostsMPI(
        MpiGridCell& cell,
        int my_rank,
        int current_step,
        const MpiSimConfig& cfg,
        std::unordered_map<int, FlatBody*>& primary_by_id,
        MPI_Comm comm)
    {
        int size = 0;
        MPI_Comm_size(comm, &size);

        const int num_cells = size;
        const int my_cell_index = my_rank; // 1:1 mapping

        // 1) Build local send buffers.
        cell.BuildGhostSendBuffers(
            my_cell_index,
            num_cells,
            cfg.nx,
            cfg.ny,
            cfg.world_bounds,
            cfg.cell_width,
            cfg.cell_height);

        // 2) Alltoall sizes.
        std::vector<int> send_counts(size, 0);
        std::vector<int> recv_counts(size, 0);

        for (int dst = 0; dst < size; ++dst) {
            send_counts[dst] = static_cast<int>(cell.ghost_send_buffers[dst].size());
        }

        MPI_Alltoall(
            send_counts.data(), 1, MPI_INT,
            recv_counts.data(), 1, MPI_INT,
            comm);

        // 3) Build displacements.
        std::vector<int> send_displs(size, 0);
        std::vector<int> recv_displs(size, 0);

        int total_send = 0;
        int total_recv = 0;

        for (int i = 0; i < size; ++i) {
            send_displs[i] = total_send;
            total_send += send_counts[i];
            recv_displs[i] = total_recv;
            total_recv += recv_counts[i];
        }

        std::vector<std::uint8_t> send_buffer(total_send);
        std::vector<std::uint8_t> recv_buffer(total_recv);

        // Concatenate into single send buffer.
        for (int dst = 0; dst < size; ++dst) {
            const auto& src_vec = cell.ghost_send_buffers[dst];
            if (src_vec.empty()) continue;
            std::copy(src_vec.begin(), src_vec.end(),
                send_buffer.begin() + send_displs[dst]);
        }

        // 4) Alltoallv actual payload.
        MPI_Alltoallv(
            send_buffer.data(), send_counts.data(), send_displs.data(), MPI_UNSIGNED_CHAR,
            recv_buffer.data(), recv_counts.data(), recv_displs.data(), MPI_UNSIGNED_CHAR,
            comm);

        // 5) Feed each src chunk into ReceiveGhostBuffer.
        for (int src = 0; src < size; ++src) {
            int count = recv_counts[src];
            if (!count) continue;

            const std::uint8_t* data =
                recv_buffer.data() + recv_displs[src];

            cell.ReceiveGhostBuffer(
                current_step,
                src,
                data,
                static_cast<std::size_t>(count));
        }

        // 6) Remove stale ghosts that were not touched this step.
        cell.RemoveStaleGhosts(current_step);
    }

    void ExchangeMigrationsMPI(
        MpiGridCell& cell,
        int my_rank,
        const MpiSimConfig& cfg,
        std::unordered_map<int, FlatBody*>& primary_by_id,
        MPI_Comm comm)
    {
        int size = 0;
        MPI_Comm_size(comm, &size);
        const int num_cells = size;
        const int my_cell_index = my_rank;

        // 1) Build local migration buffers (and remove migrated primaries).
        cell.BuildMigrationSendBuffers(
            my_cell_index,
            num_cells,
            cfg.nx,
            cfg.ny,
            cfg.world_bounds,
            cfg.cell_width,
            cfg.cell_height,
            primary_by_id);

        // 2) Alltoall sizes.
        std::vector<int> send_counts(size, 0);
        std::vector<int> recv_counts(size, 0);

        for (int dst = 0; dst < size; ++dst) {
            send_counts[dst] = static_cast<int>(cell.migration_send_buffers[dst].size());
        }

        MPI_Alltoall(
            send_counts.data(), 1, MPI_INT,
            recv_counts.data(), 1, MPI_INT,
            comm);

        // 3) Displacements.
        std::vector<int> send_displs(size, 0);
        std::vector<int> recv_displs(size, 0);

        int total_send = 0;
        int total_recv = 0;
        for (int i = 0; i < size; ++i) {
            send_displs[i] = total_send;
            total_send += send_counts[i];
            recv_displs[i] = total_recv;
            total_recv += recv_counts[i];
        }

        std::vector<std::uint8_t> send_buffer(total_send);
        std::vector<std::uint8_t> recv_buffer(total_recv);

        for (int dst = 0; dst < size; ++dst) {
            const auto& src_vec = cell.migration_send_buffers[dst];
            if (src_vec.empty()) continue;
            std::copy(src_vec.begin(), src_vec.end(),
                send_buffer.begin() + send_displs[dst]);
        }

        // 4) Alltoallv payload.
        MPI_Alltoallv(
            send_buffer.data(), send_counts.data(), send_displs.data(), MPI_UNSIGNED_CHAR,
            recv_buffer.data(), recv_counts.data(), recv_displs.data(), MPI_UNSIGNED_CHAR,
            comm);

        // 5) Apply incoming migrations.
        for (int src = 0; src < size; ++src) {
            int count = recv_counts[src];
            if (!count) continue;

            const std::uint8_t* data =
                recv_buffer.data() + recv_displs[src];

            cell.ReceiveMigrationBuffer(
                my_cell_index,
                data,
                static_cast<std::size_t>(count),
                primary_by_id);
        }
    }

} // namespace FlatPhysics
