<template>
  <div class="about">
    <h1>This is an about page</h1>
    <div v-if="loading">Loading...</div>
    <div v-else>
      <h2>Results from /get/1:</h2>
      <pre>{{ formattedResult }}</pre>
    </div>
  </div>
</template>

<script>
export default {
  data() {
    return {
      loading: true,
      result: null,
    };
  },
  computed: {
    formattedResult() {
      // Parse the result string and format it
      const rows = this.result.split('\n').filter(row => row.trim() !== '');

      // Display each row on a new line
      return rows.join('\n');
    },
  },
  mounted() {
    // Fetch data from the server
    this.fetchData();
  },
  methods: {
    fetchData() {
      // Use Fetch API or Axios to make a GET request to the server
      fetch('http://localhost:3000/get/1')
        .then((response) => response.text())
        .then((data) => {
          // Update the component state with the fetched data
          this.loading = false;
          this.result = data;
        })
        .catch((error) => {
          console.error('Error fetching data:', error);
          this.loading = false;
          this.result = 'Error fetching data';
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
}
</style>
