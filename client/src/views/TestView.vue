<template>
    <div class="about">
        <h1>This is an about page</h1>
        <div v-if="loading">Loading...</div>
        <div v-else>
            <h2>Results from /get/1:</h2>
            <template v-if="result && result.data && result.data.length > 0">
                <table>
                    <tbody>
                        <tr v-for="row in sortedResult" :key="row.task_id">
                            <td>{{ row.task_id }}</td>
                            <td>{{ row.task }}</td>
                            <td>{{ row.day_id }}</td>
                            <td>{{ formatTime(row.start_datetime) }}</td>
                            <td>{{ formatTime(row.end_datetime) }}</td>
                            <td>
                                <button @click="editRow(row)">Edit</button>
                            </td>
                        </tr>
                    </tbody>
                </table>
            </template>
            <template v-else>
                No elements found.
            </template>


            <!-- Edit Form -->
            <div class="request-form" v-if="editingRow">
                <h2>Edit Data</h2>

                <!-- Input field for each column -->
                <div v-if="editingRow" v-for="(value, key) in editingRow" :key="key">
                    <label :for="`${key}EditInput`">{{ key }}:</label>
                    <input v-if="editingRow[key] !== null && editingRow[key] !== undefined" v-model="editingRow[key]"
                        :id="`${key}EditInput`" />
                </div>

                <!-- Dropdown for HTTP Method -->
                <div>
                    <label for="httpMethodEdit">HTTP Method:</label>
                    <select v-model="selectedMethodEdit" id="httpMethodEdit">
                        <option value="PUT">PUT</option>
                        <option value="DELETE">DELETE</option>
                    </select>
                </div>

                <button @click="updateData">Update Data</button>
                <button @click="deleteData">Delete Data</button>

            </div>

        </div>
    </div>
</template>
  
<script>
export default {
    data() {
        return {
            loading: true,
            result: null,
            jsonInput: '',
            selectedMethod: 'POST',
            selectedMethodInsert: 'POST',
            selectedMethodEdit: 'PUT',
            taskInput: '',
            taskIdInput: '',
            startDatetimeInput: '',
            endDatetimeInput: '',
            dayIdInput: '',
            updatedResult: null,
            editingRow: null,
        };
    },
    mounted() {
        this.fetchData();
    },
    computed: {
        sortedResult() {
            return (this.result && this.result.data && this.result.data.length > 0) ?
                this.result.data.slice().sort((a, b) => {
                    // Convert the date strings to Date objects for proper comparison
                    const dateA = new Date(a.start_datetime);
                    const dateB = new Date(b.start_datetime);

                    // Sort in descending order (most recent to latest)
                    return dateB - dateA;
                }) :
                [];
        },
    },
    methods: {
        fetchData() {
            fetch('http://localhost:3000/get/1')
                .then((response) => {
                    if (!response.ok) {
                        throw new Error(`HTTP error! Status: ${response.status}`);
                    }
                    return response.json();
                })
                .then((data) => {
                    this.loading = false;
                    this.result = data;
                })
                .catch((error) => {
                    console.error('Error fetching data:', error);
                    this.loading = false;
                    this.result = { data: [] }; // Provide an empty data array to prevent further issues
                });
        },

        formatDate(dateString) {
            const date = new Date(dateString);
            const options = { hour: 'numeric', minute: 'numeric', hour12: false };
            return date.toLocaleDateString('en-US', options);
        },
        formatTime(dateTimeString) {
            const date = new Date(dateTimeString);
            const options = { hour: 'numeric', minute: 'numeric', hour12: false };
            return date.toLocaleTimeString('en-US', options);
        },
        editRow(row) {
            // Make a copy of the row for editing
            this.editingRow = { ...row };
        },
        updateData() {
            const fullEndpoint = `http://localhost:3000/update/1`;

            // Get the column names from the server response (assuming it provides column names)
            const columnNames = Object.keys(this.editingRow);

            // Use column names in requestData
            const requestData = {
                columns: columnNames,
                values: Object.values(this.editingRow),
                where: `task_id='${this.editingRow.task_id}' AND day_id='${this.editingRow.day_id}'`,
            };

            console.log('requestData:', requestData);

            const requestOptions = {
                method: this.selectedMethodEdit,
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(requestData),
            };

            console.log('requestOptions:', requestOptions);

            fetch(fullEndpoint, requestOptions)
                .then((response) => response.json())
                .then((data) => {
                    console.log('Response from server:', data);

                    this.updatedResult = data;
                    // Update the table data after a successful request
                    this.fetchData();
                    // Clear the editing state
                    this.editingRow = null;
                })
                .catch((error) => {
                    console.error('Error updating data:', error);
                    this.updatedResult = 'Error updating data';
                });
        },

        deleteData() {
            const fullEndpoint = `http://localhost:3000/delete/1`;

            const requestData = {
                where: `task_id='${this.editingRow.task_id}' AND day_id='${this.editingRow.day_id}'`,
            };

            const requestOptions = {
                method: 'DELETE', // Change to DELETE method
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(requestData),
            };

            fetch(fullEndpoint, requestOptions)
                .then((response) => {
                    // Check if the response is successful
                    if (!response.ok) {
                        throw new Error(`HTTP error! Status: ${response.status}`);
                    }
                    // Check if the response is valid JSON
                    return response.json();
                })
                .then((data) => {
                    this.updatedResult = data;
                    // Update the table data after a successful request
                    this.fetchData();
                    // Clear the editing state
                    this.editingRow = null;
                })
                .catch((error) => {
                    console.error('Error deleting data:', error);
                    this.updatedResult = 'Error deleting data';
                });
        },
    },
};
</script>
  
<style>
@media (min-width: 1024px) {
    .about {
        min-height: 100vh;
        display: flex;
        align-items: center;
        flex-direction: column;
    }

    table {
        width: 80%;
        margin-top: 20px;
        border-collapse: collapse;
    }

    th,
    td {
        border: 1px solid #dddddd;
        text-align: left;
        padding: 8px;
    }

    th {
        background-color: #f2f2f2;
    }

    .request-form {
        margin-top: 20px;
    }
}
</style>
  