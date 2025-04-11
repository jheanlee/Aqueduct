import {Container, Grid, GridItem, Text, Alert, Box} from "@chakra-ui/react";
import {useEffect, useState} from "react";
import {getStatus} from "../services/client.ts";

function Dashboard() {
  const [flagStatusError, setFlagStatusError] = useState<boolean>(false);
  const [uptime, setUptime] = useState<number>(0);
  const [tunnelServiceUp, setTunnelService] = useState<boolean>(false);
  const [apiServiceUp, setApiService] = useState<boolean>(false);
  const [connectedClients, setConnectedClients] = useState<number>(0);

  useEffect(() => {
    (async () => {
      const res = await getStatus();
      if (res !== null) {
        setFlagStatusError(false);
        setUptime(res["uptime"]);
        setTunnelService(res["tunnel_service_up"]);
        setApiService(res["api_service_up"]);
        setConnectedClients(res["connected_clients"]);
      } else {
        setFlagStatusError(true);
      }
    })();
  }, [])

  return (
    <Container w="100%" h="100%" p="1rem">
      <Grid w="100%" h="100%" gap={5} p={10}>
        <GridItem rowSpan={1} colSpan={1}>
          <Alert.Root status="error" hidden={!flagStatusError}>
            <Alert.Indicator/>
            <Alert.Content>
              <Alert.Title>Connection Lost</Alert.Title>
              <Alert.Description>
                Unable to connnect to api server
              </Alert.Description>
            </Alert.Content>
          </Alert.Root>
          <Box p={10} bg="bg.muted" hidden={flagStatusError}>
            <Text textStyle="xl" fontWeight="semibold">Overview</Text>
            <Text>{"Uptime: " + uptime.toString()}</Text>{/*TODO time format*/}
            <Text>{"Tunnel Service: " + ((tunnelServiceUp) ? "Online" : "Offline")}</Text>
            <Text>{"API Service: " + ((apiServiceUp) ? "Online" : "Offline")}</Text>
            <Text>{"Connected Clients: " + connectedClients.toString()}</Text>
          </Box>
        </GridItem>
      </Grid>
    </Container>
  )
}

export default Dashboard;