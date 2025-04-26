import { Box, Button, Flex, Spacer } from "@chakra-ui/react";
import { Link } from "react-router";

function Navbar() {
  return (
    <Flex minWidth="max-content" alignItems="center" gap="2px" h="3.5rem">
      <Box>
        <Button variant="plain">
          <Link to="/">Dashboard</Link>
        </Button>
      </Box>
      <Box>
        <Button variant="plain">
          <Link to="/client">Clients</Link>
        </Button>
      </Box>
      <Box>
        <Button variant="plain">
          <Link to="/access">Access</Link>
        </Button>
      </Box>

      <Spacer />

      <Button m={2}>
        <Link to="/login">login</Link>
      </Button>
    </Flex>
  );
}

export default Navbar;
